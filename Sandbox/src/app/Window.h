#pragma once

#include "string"
#include <functional>

//allows us to not include the entire window library header
struct GLFWwindow;

//the window initializer properties
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
	//makes code more readable
	using EventCallbackFn = std::function<void()>;


	Window(const WindowProps& props);
	~Window();

	//updates window frames
	void OnUpdate() ;

	unsigned int GetWidth() const  { return m_Data.Width; }
	unsigned int GetHeight() const  { return m_Data.Height; }

	//Window attributes
	void SetEventCallback(const EventCallbackFn& callback)  { m_Data.CloseApplication = callback; }

	void* GetNativeWindow() const { return m_Window; }
private:
	void Init(const WindowProps& props);
	void ShutDown();
private:
	GLFWwindow* m_Window;

	struct WindowData
	{
		std::string Title;
		unsigned int Width = -1, Height = -1;

		EventCallbackFn CloseApplication;
	};

	WindowData m_Data;
};