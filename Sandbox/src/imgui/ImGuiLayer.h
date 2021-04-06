#pragma once


#include "app/application.h"

class ImGuiStartup
{
public:
	ImGuiStartup(Application& app);
	~ImGuiStartup();

	void Begin();
	void End();
private:
	Application& app;
};