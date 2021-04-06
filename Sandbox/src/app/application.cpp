#include "Application.h"


#define BIND_EVENT_FN(x) std::bind(&Application::x, this)


Application::Application(const WindowProps& props)
{
	//makes new window
	m_Window = NewRef<Window>(props);
	//only event we need to monitor is closing the window
	m_Window->SetEventCallback((const Window::EventCallbackFn)(BIND_EVENT_FN(OnWindowClose)));
}

//update frames
void Application::UpdateWindow()
{
	m_Window->OnUpdate();
}

//closes window
void Application::OnWindowClose()
{
	m_Running = false;
}