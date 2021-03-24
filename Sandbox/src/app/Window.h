#pragma once

#include "Events\Event.h"
#include "string"
#include <functional>

//#include <D:\Hazel\Hazel\vendor\GLFW\include\GLFW\glfw3.h>
struct GLFWwindow;
struct WindowProps
{
	std::string Title;
	unsigned int Width;
	unsigned int Height;

	WindowProps(const std::string& title = "Hazel Engine",
		unsigned int width = 900,
		unsigned int height = 500)
		:Title(title), Width(width), Height(height)
	{
	}
};

class Window
{
public:
	using EventCallbackFn = std::function<void(Event&)>;


	Window(const WindowProps& props);
	virtual ~Window();

	void OnUpdate() ;

	inline unsigned int GetWidth() const  { return m_Data.Width; }
	inline unsigned int GetHeight() const  { return m_Data.Height; }
	inline float GetAspectRatio() const  { return (float)m_Data.Width / (float)m_Data.Height; }

	//Window attributes
	inline void SetEventCallback(const EventCallbackFn& callback)  { m_Data.EventCallback = callback; }
	void SetVSync(bool enabled) ;
	bool IsVSync() const ;

	inline virtual void* GetNativeWindow() const { return m_Window; }
private:
	virtual void Init(const WindowProps& props);
	virtual void ShutDown();
private:
	GLFWwindow* m_Window;

	struct WindowData
	{
		std::string Title;
		unsigned int Width = -1, Height = -1;
		bool VSync = false;

		EventCallbackFn EventCallback;
	};

	WindowData m_Data;
};