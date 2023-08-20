#pragma once

#include <GLFW/glfw3.h>
#include <string>

namespace scex::ImGuiController
{
	void Setup(GLFWwindow* window);
	void Setup(GLFWwindow* window, const std::string& fileToOpen);
	bool HasControl();
	void OnPathsDropped(const char** paths, int pathCount);
	void Tick(double deltaTime);
}