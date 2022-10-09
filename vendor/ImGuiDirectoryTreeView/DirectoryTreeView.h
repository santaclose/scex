#include <imgui.h>

#include <string>

namespace DirectoryTreeView
{
	typedef void (*onFileClickCallback)(const std::string& filePath);
	typedef void (*onContextMenuCallback)(const std::string& path);

	void SetOnFileClickCallback(onFileClickCallback callback);
	void AddFileContextMenuOption(const std::string& name, onContextMenuCallback callback);
	void AddFolderContextMenuOption(const std::string& name, onContextMenuCallback callback);
	bool OnImGui(const std::string& directoryPath, const std::string& panelName);
}