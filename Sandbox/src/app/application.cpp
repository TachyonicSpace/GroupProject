#include "Application.h"

#include "Events\input.h"
#include "imgui\ImGuiLayer.h"

//#include <D:\Hazel\Hazel\vendor\GLFW\include\GLFW\glfw3.h>


#define BIND_EVENT_FN(x) std::bind(&Application::x, this, std::placeholders::_1)


Application::Application(const WindowProps& props)
{
	m_Window = NewRef<Window>(props);
	m_Window->SetEventCallback(BIND_EVENT_FN(OnEvent));
	Input::app = this;
}

void Application::OnEvent(Event& e)
{

	EventDispatcher dispatcher(e);
	dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FN(OnWindowClose));
	dispatcher.Dispatch<WindowResizeEvent>(BIND_EVENT_FN(OnWindowResize));

	imgs->OnEvent(e);
}

Application::~Application()
{
}

void Application::Run()
{
	m_Window->OnUpdate();
}

bool Application::OnWindowClose(WindowCloseEvent& e)
{
	m_Running = false;
	return true;
}

bool Application::OnWindowResize(WindowResizeEvent& e)
{
	return true;
}