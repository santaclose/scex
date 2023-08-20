#pragma once

#include <string>
#include <vector>
#include <GLFW/glfw3.h>

namespace Utils
{
	std::wstring Utf8ToWstring(const std::string& str);
	void SetWindowIcon(const std::string& path, GLFWwindow* window);

	int SubprocessCall(const std::string& cmd);
	int SubprocessCall(const std::vector<std::string>& cmd);
	std::string SubprocessCheckOutput(const std::vector<std::string>& cmd);

	void ShowInFileExplorer(const std::string& path);
	void OpenInFileExplorer(const std::string& folderPath);
}