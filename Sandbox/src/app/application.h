#pragma once

#include "Window.h"
#include "Events\ApplicationEvent.h"

template<typename T>
using Scope = std::unique_ptr<T>;
template<typename T, typename ... Args>
constexpr Scope<T> NewScope(Args&& ... args)
{
	return std::make_unique<T>(std::forward<Args>(args)...);
}

template<typename T>
using Ref = std::shared_ptr<T>;
template<typename T, typename ... Args>
constexpr Ref<T> NewRef(Args&& ... args)
{
	return std::make_shared<T>(std::forward<Args>(args)...);
}

class ImGuiStartup;

class Application {
public:
	Application(const WindowProps& props = {});
	virtual ~Application();

	void Run();

	void OnEvent(Event& e);

	Window& GetWindow() { return *m_Window; }

	void Close() { m_Running = false; }

	bool m_Running = true;
	ImGuiStartup* imgs;
private:
	bool OnWindowClose(WindowCloseEvent& e);
	bool OnWindowResize(WindowResizeEvent& e);
private:
	Ref<Window> m_Window;
};