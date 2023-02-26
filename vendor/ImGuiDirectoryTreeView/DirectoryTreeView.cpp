#include "DirectoryTreeView.h"

#include <filesystem>
#include <unordered_map>

// adaptation of samyuu's code at https://github.com/ocornut/imgui/issues/5137
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
	OnFileClickCallback fileClickCallback = nullptr;

	bool isHoveringNodeThisFrame = false;
	const DirectoryNode* lastHoveredNode = nullptr;
	std::vector<std::pair<std::string, OnContextMenuCallback>> fileContextMenuOptions;
	std::vector<std::pair<std::string, OnContextMenuCallback>> folderContextMenuOptions;

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

	DirectoryNode CreateDirectoryNodeTreeFromPath(const std::filesystem::path& rootPath)
	{
		DirectoryNode rootNode;
		rootNode.FullPath = rootPath.u8string();
		rootNode.FileName = rootPath.filename().u8string();
		if (rootNode.IsDirectory = std::filesystem::is_directory(rootPath); rootNode.IsDirectory)
			RecursivelyAddDirectoryNodes(rootNode, std::filesystem::directory_iterator(rootPath));

		return rootNode;
	}
	
	void SetLastHoveredNode(const DirectoryNode* node)
	{
		lastHoveredNode = node;
		isHoveringNodeThisFrame = true;
	}

	void RecursivelyDisplayDirectoryNode(const DirectoryNode& parentNode)
	{
		ImGui::PushID(&parentNode);
		if (parentNode.IsDirectory)
		{
			if (ImGui::TreeNodeEx(parentNode.FileName.c_str(), ImGuiTreeNodeFlags_SpanFullWidth))
			{
				if (ImGui::IsItemHovered())
					SetLastHoveredNode(&parentNode);
				for (const DirectoryNode& childNode : parentNode.Children)
					RecursivelyDisplayDirectoryNode(childNode);
				ImGui::TreePop();
			}
			else
			{
				if (ImGui::IsItemHovered())
					SetLastHoveredNode(&parentNode);
			}
		}
		else
		{
			if (ImGui::TreeNodeEx(parentNode.FileName.c_str(), ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_SpanFullWidth))
			{
				if (ImGui::IsItemClicked(0))
				{
					if (fileClickCallback != nullptr)
						fileClickCallback(parentNode.FullPath);
				}
			}
			if (ImGui::IsItemHovered())
				SetLastHoveredNode(&parentNode);
		}
		ImGui::PopID();
	}
}

void DirectoryTreeView::SetOnFileClickCallback(OnFileClickCallback callback)
{
	fileClickCallback = callback;
}

void DirectoryTreeView::AddFileContextMenuOption(const std::string& name, OnContextMenuCallback callback)
{
	fileContextMenuOptions.push_back({ name, callback });
}

void DirectoryTreeView::AddFolderContextMenuOption(const std::string& name, OnContextMenuCallback callback)
{
	folderContextMenuOptions.push_back({ name, callback });
}

bool DirectoryTreeView::OnImGui(const std::string& directoryPath, const std::string& panelName)
{
	bool windowIsOpen = true;
	if (treePerDirectory.find(directoryPath) == treePerDirectory.end())
		treePerDirectory[directoryPath] = CreateDirectoryNodeTreeFromPath(directoryPath);

	if (ImGui::Begin(panelName.c_str(), &windowIsOpen, ImGuiWindowFlags_NoSavedSettings))
	{
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 0.0f, 0.0f });
		isHoveringNodeThisFrame = false;
		RecursivelyDisplayDirectoryNode(treePerDirectory[directoryPath]);

		if (lastHoveredNode != nullptr)
		{
			const auto& vectorToUse = lastHoveredNode->IsDirectory ? folderContextMenuOptions : fileContextMenuOptions;
			if (ImGui::IsMouseDown(1) && isHoveringNodeThisFrame)
			{
				if (vectorToUse.size() > 0)
					ImGui::OpenPopup(lastHoveredNode->IsDirectory ? "folder_right_click_popup" : "file_right_click_popup");
			}
			if (vectorToUse.size() > 0 && ImGui::BeginPopup(lastHoveredNode->IsDirectory ? "folder_right_click_popup" : "file_right_click_popup"))
			{
				for (auto& item : vectorToUse)
				{
					if (ImGui::Selectable(item.first.c_str()))
					{
						item.second(lastHoveredNode->IsDirectory ? lastHoveredNode->FullPath : lastHoveredNode->FullPath);
						ImGui::CloseCurrentPopup();
					}
				}
				ImGui::EndPopup();
			}
		}

		ImGui::PopStyleVar();
	}
	ImGui::End();
	return windowIsOpen;
}