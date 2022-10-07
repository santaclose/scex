#include <imgui.h>

#include <string>

namespace DirectoryTreeView
{
	typedef void (*onFileClickCallback)(const std::string& filePath);

	void SetOnFileClickCallback(onFileClickCallback callback);
	bool OnImGui(const std::string& directoryPath, const std::string& panelName);
}