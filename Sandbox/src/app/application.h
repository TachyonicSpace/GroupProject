#pragma once

#include "Window.h"

//allows easier reading of making new shared pointers
template<typename T>
using Ref = std::shared_ptr<T>;
template<typename T, typename ... Args>
constexpr Ref<T> NewRef(Args&& ... args)
{
	return std::make_shared<T>(std::forward<Args>(args)...);
}

class Application {
public:
	Application(const WindowProps& props = {});
	~Application() = default;

	void UpdateWindow();

	Window& GetWindow() { return *m_Window; }

	void Close() { m_Running = false; }

	bool m_Running = true;
private:
	void OnWindowClose();
private:
	Ref<Window> m_Window;
};