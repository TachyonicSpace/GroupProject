//allows us to hide console window
#include "windows.h"

#include "Window.h"
#include "iostream"

#include <glad/glad.h>

#include <GLFW\glfw3.h>

static bool s_GLFWInitilized = false;

static void GLFWErrorCallback(int error, const char* description)
{
	std::cout << "GLFW Error (" << error << ", " << description << ")\n";
}

Window::Window(const WindowProps& props)
{
	Init(props);
}

Window::~Window()
{
	ShutDown();
}

void Window::Init(const WindowProps& props)
{
	m_Data.Title = props.Title;
	m_Data.Width = props.Width;
	m_Data.Height = props.Height;

	//hides console window since we don't need it
	ShowWindow(GetConsoleWindow(), SW_HIDE);

	//initialize window
	if (!s_GLFWInitilized)
	{
		int success = glfwInit();
		if (!success)
		{
			std::cout << "GLFW failed to initiate";
			return;
		}
		glfwSetErrorCallback(GLFWErrorCallback);

		s_GLFWInitilized = true;
	}
	m_Window = glfwCreateWindow((int)props.Width, (int)props.Height, m_Data.Title.c_str(), nullptr, nullptr);

	//initializing opengl
	{
		glfwMakeContextCurrent(m_Window);
		int status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
		if (!status)
		{
			std::cout << "glad failed to initiate";
			return;
		}
	}

	//set the current window
	glfwSetWindowUserPointer(m_Window, &m_Data);
	//sets framerate to a max of ~60fps
	glfwSwapInterval(1);


	//closes program
	glfwSetWindowCloseCallback(m_Window, [](GLFWwindow* window)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
			data.CloseApplication();
		});
}


void Window::ShutDown()
{
	glfwDestroyWindow(m_Window);
}

void Window::OnUpdate()
{
	glfwPollEvents();
	glfwSwapBuffers(m_Window);
}