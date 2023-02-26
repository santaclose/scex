#pragma once

#include <imgui.h>
#include <string>

namespace DirectoryTreeView
{
	typedef void (*OnFileClickCallback)(const std::string& filePath);
	typedef void (*OnContextMenuCallback)(const std::string& path);

	void SetOnFileClickCallback(OnFileClickCallback callback);
	void AddFileContextMenuOption(const std::string& name, OnContextMenuCallback callback);
	void AddFolderContextMenuOption(const std::string& name, OnContextMenuCallback callback);
	bool OnImGui(const std::string& directoryPath, const std::string& panelName);
}