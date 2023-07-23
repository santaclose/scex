#pragma once

#include <GLFW/glfw3.h>
#include <string>

struct ImGuiTestEngine;

namespace ste::ImGuiController
{
	void Setup(GLFWwindow* window);
	void Setup(GLFWwindow* window, const std::string& fileToOpen);
	bool HasControl();
	void Tick();

	void ConsoleLog(const std::string& log);
	void ShowConsole();
	void RegisterTestEngineTests(ImGuiTestEngine* e);
}