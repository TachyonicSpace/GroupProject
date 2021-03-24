
#pragma once

#include "KeyCodes.h"
#include "MousebuttonCodes.h"

class Application;

class Input
{
public:
	static Application* app;

	static bool IsKeyPressed(KeyCode key);

	static bool IsMouseButtonPressed(MouseCode button);
	static std::pair<float, float> GetMousePos();
	static float GetMouseX();
	static float GetMouseY();
};