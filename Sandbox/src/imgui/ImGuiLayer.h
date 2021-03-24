#pragma once


#include "app/application.h"

class ImGuiStartup
{
public:
	ImGuiStartup(Application& app);
	~ImGuiStartup();

	bool OnEvent(Event& e);
	void OnImGuiRender();

	void Begin();
	void End(Application& app);
private:
};