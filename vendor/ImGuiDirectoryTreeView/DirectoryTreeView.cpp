#include "DirectoryTreeView.h"

#include <filesystem>
#include <unordered_map>

// taken from samyuu at https://github.com/ocornut/imgui/issues/5137
namespace DirectoryTreeView
{
	struct DirectoryNode
	{
		std::string FullPath;
		std::string FileName;
		std::vector<DirectoryNode> Children;
		bool IsDirectory;
	};

	std::unordered_map<std::string, DirectoryNode> treePerDirectory;
	onFileClickCallback fileClickCallback = nullptr;

	void RecursivelyAddDirectoryNodes(DirectoryNode& parentNode, std::filesystem::directory_iterator directoryIterator)
	{
		for (const std::filesystem::directory_entry& entry : directoryIterator)
		{
			DirectoryNode& childNode = parentNode.Children.emplace_back();
			childNode.FullPath = entry.path().u8string();
			childNode.FileName = entry.path().filename().u8string();
			if (childNode.IsDirectory = entry.is_directory(); childNode.IsDirectory)
				RecursivelyAddDirectoryNodes(childNode, std::filesystem::directory_iterator(entry));
		}

		auto moveDirectoriesToFront = [](const DirectoryNode& a, const DirectoryNode& b) { return (a.IsDirectory > b.IsDirectory); };
		std::sort(parentNode.Children.begin(), parentNode.Children.end(), moveDirectoriesToFront);
	}

	DirectoryNode CreateDirectryNodeTreeFromPath(const std::filesystem::path& rootPath)
	{
		DirectoryNode rootNode;
		rootNode.FullPath = rootPath.u8string();
		rootNode.FileName = rootPath.filename().u8string();
		if (rootNode.IsDirectory = std::filesystem::is_directory(rootPath); rootNode.IsDirectory)
			RecursivelyAddDirectoryNodes(rootNode, std::filesystem::directory_iterator(rootPath));

		return rootNode;
	}

	void RecursivelyDisplayDirectoryNode(const DirectoryNode& parentNode)
	{
		ImGui::PushID(&parentNode);
		if (parentNode.IsDirectory)
		{
			if (ImGui::TreeNodeEx(parentNode.FileName.c_str(), ImGuiTreeNodeFlags_SpanFullWidth))
			{
				for (const DirectoryNode& childNode : parentNode.Children)
					RecursivelyDisplayDirectoryNode(childNode);
				ImGui::TreePop();
			}
		}
		else
		{
			if (ImGui::TreeNodeEx(parentNode.FileName.c_str(), ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_SpanFullWidth))
			{
				if (ImGui::IsItemClicked())
				{
					if (fileClickCallback != nullptr)
						fileClickCallback(parentNode.FullPath);
				}
			}
		}
		ImGui::PopID();
	}
}

void DirectoryTreeView::SetOnFileClickCallback(onFileClickCallback callback)
{
	fileClickCallback = callback;
}

bool DirectoryTreeView::OnImGui(const std::string& directoryPath, const std::string& panelName)
{
	bool windowIsOpen = true;
	if (treePerDirectory.find(directoryPath) == treePerDirectory.end())
		treePerDirectory[directoryPath] = CreateDirectryNodeTreeFromPath(directoryPath);

	if (ImGui::Begin(panelName.c_str(), &windowIsOpen, ImGuiWindowFlags_NoSavedSettings))
	{
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 0.0f, 0.0f });
		RecursivelyDisplayDirectoryNode(treePerDirectory[directoryPath]);
		ImGui::PopStyleVar();
	}
	ImGui::End();
	return windowIsOpen;
}