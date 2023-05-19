#pragma once

#include <GLFW/glfw3.h>
#include <string>

namespace ste::ImGuiController
{
	void Setup(GLFWwindow* window);
	void Setup(GLFWwindow* window, const std::string& fileToOpen);
	bool HasControl();
	void Tick();
}