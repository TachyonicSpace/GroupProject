#include "ImGuiLayer.h"

#define IMGUI_IMPL_API
#define IMGUI_IMPL_OPENGL_LOADER_GLAD
#include "imgui.h"
#include "examples/imgui_impl_glfw.h"
#include "examples/imgui_impl_opengl3.h"

//temp
#include <GLFW\glfw3.h>


ImGuiStartup::ImGuiStartup(Application& app)
{
	//setup Imgui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; //enables keyboard controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad; //enables Gamepad controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; //enables docking
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; //enables multi-viewport / platform windows
	//io.ConfigFlags |= ImGuiConfigFlags_ViewportsNoTaskBarIcons;
	//io.ConfigFlags |= ImGuiConfigFlags_VeiwPortsNoMerge;

	//setup ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsClassic();

	//when view ports are enabled we teak window rounding so platform window looks identical
	ImGuiStyle& style = ImGui::GetStyle();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		style.WindowRounding = 0.f;
		style.Colors[ImGuiCol_WindowBg].w = 1.f;
	}
	GLFWwindow* window = static_cast<GLFWwindow*>(app.GetWindow().GetNativeWindow());

	// setup platform/rendererBindings
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 410");
}

ImGuiStartup::~ImGuiStartup()
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplOpenGL3_Shutdown();
	ImGui::DestroyContext();
}

void ImGuiStartup::OnImGuiRender()
{
	//static bool show = true;
	//ImGui::ShowDemoWindow(&show);
}

bool ImGuiStartup::OnEvent(Event& e)
{
	auto io = ImGui::GetIO();
	e.handled |= e.IsInCategory(EventCategory::EventCategoryMouse) & io.WantCaptureMouse;
	e.handled |= e.IsInCategory(EventCategory::EventCategoryKeyboard) & io.WantCaptureKeyboard;
	return true;
}

void ImGuiStartup::Begin()
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
}

void ImGuiStartup::End(Application& app)
{
	ImGuiIO& io = ImGui::GetIO();
	io.DisplaySize = ImVec2((float)app.GetWindow().GetWidth(), (float)app.GetWindow().GetHeight());

	//rendering
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		GLFWwindow* backupCurrentContext = glfwGetCurrentContext();
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
		glfwMakeContextCurrent(backupCurrentContext);
	}
}