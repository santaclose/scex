#pragma once

#include <imgui.h>
#include <string>
#include <vector>
#include <thread>
#include <filesystem>

struct DirectoryTreeViewNode
{
	std::string fullPath;
	std::string fileName;
	std::vector<DirectoryTreeViewNode> children;
	bool isDirectory;
};

struct DirectoryTreeView
{
	typedef void (*OnFileClickCallback)(const std::string& filePath);
	typedef void (*OnContextMenuCallback)(const std::string& path);

	std::string panelName = "Folder search";
	std::string directoryPath;

	DirectoryTreeView(const std::string& folderPath, OnFileClickCallback fileClickCallback = nullptr,
		std::vector<std::pair<std::string, OnContextMenuCallback>>* fileContextMenuOptions = nullptr,
		std::vector<std::pair<std::string, OnContextMenuCallback>>* folderContextMenuOptions = nullptr);
	bool OnImGui();

private:
	DirectoryTreeViewNode directoryTreeRoot;
	OnFileClickCallback fileClickCallback = nullptr;

	bool isHoveringNodeThisFrame = false;
	const DirectoryTreeViewNode* lastHoveredNode = nullptr;
	std::vector<std::pair<std::string, OnContextMenuCallback>>* fileContextMenuOptions;
	std::vector<std::pair<std::string, OnContextMenuCallback>>* folderContextMenuOptions;

	void RecursivelyAddDirectoryNodes(DirectoryTreeViewNode& parentNode, std::filesystem::directory_iterator directoryIterator);
	DirectoryTreeViewNode CreateDirectoryNodeTreeFromPath(const std::filesystem::path& rootPath);
	void SetLastHoveredNode(const DirectoryTreeViewNode* node);
	void RecursivelyDisplayDirectoryNode(const DirectoryTreeViewNode& parentNode);
};