#pragma once

#include <string>
#include <GLFW/glfw3.h>

namespace Utils
{
	std::wstring Utf8ToWstring(const std::string& str);
	void SetWindowIcon(const std::string& path, GLFWwindow* window);
}