#include "DirectoryTreeView.h"

DirectoryTreeView::DirectoryTreeView(const std::string& folderPath, OnFileClickCallback fileClickCallback, std::vector<std::pair<std::string, OnContextMenuCallback>>* fileContextMenuOptions, std::vector<std::pair<std::string, OnContextMenuCallback>>* folderContextMenuOptions)
{
	panelName = "Folder view##" + std::to_string((int)this);
	directoryPath = folderPath;
	this->fileClickCallback = fileClickCallback;
	this->fileContextMenuOptions = fileContextMenuOptions;
	this->folderContextMenuOptions = folderContextMenuOptions;
	directoryTreeRoot = CreateDirectoryNodeTreeFromPath(folderPath);
}

bool DirectoryTreeView::OnImGui()
{
	bool windowIsOpen = true;
	if (ImGui::Begin(panelName.c_str(), &windowIsOpen, ImGuiWindowFlags_NoSavedSettings))
	{
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 0.0f, 0.0f });
		isHoveringNodeThisFrame = false;
		RecursivelyDisplayDirectoryNode(directoryTreeRoot);

		if (lastHoveredNode != nullptr)
		{
			const auto* vectorToUse = lastHoveredNode->isDirectory ? folderContextMenuOptions : fileContextMenuOptions;
			if (ImGui::IsMouseDown(1) && isHoveringNodeThisFrame)
			{
				if (vectorToUse->size() > 0)
					ImGui::OpenPopup(lastHoveredNode->isDirectory ? "folder_right_click_popup" : "file_right_click_popup");
			}
			if (vectorToUse->size() > 0 && ImGui::BeginPopup(lastHoveredNode->isDirectory ? "folder_right_click_popup" : "file_right_click_popup"))
			{
				for (auto& item : *vectorToUse)
				{
					if (ImGui::Selectable(item.first.c_str()))
					{
						item.second(lastHoveredNode->fullPath, id);
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

void DirectoryTreeView::RecursivelyAddDirectoryNodes(DirectoryTreeViewNode& parentNode, std::filesystem::directory_iterator directoryIterator)
{
	for (const std::filesystem::directory_entry& entry : directoryIterator)
	{
		DirectoryTreeViewNode& childNode = parentNode.children.emplace_back();
		childNode.fullPath = entry.path().u8string();
		childNode.fileName = entry.path().filename().u8string();
		if (childNode.isDirectory = entry.is_directory(); childNode.isDirectory)
			RecursivelyAddDirectoryNodes(childNode, std::filesystem::directory_iterator(entry));
	}

	auto moveDirectoriesToFront = [](const DirectoryTreeViewNode& a, const DirectoryTreeViewNode& b) { return (a.isDirectory > b.isDirectory); };
	std::sort(parentNode.children.begin(), parentNode.children.end(), moveDirectoriesToFront);
}

DirectoryTreeViewNode DirectoryTreeView::CreateDirectoryNodeTreeFromPath(const std::filesystem::path& rootPath)
{
	DirectoryTreeViewNode rootNode;
	rootNode.fullPath = rootPath.u8string();
	rootNode.fileName = rootPath.filename().u8string();
	if (rootNode.isDirectory = std::filesystem::is_directory(rootPath); rootNode.isDirectory)
		RecursivelyAddDirectoryNodes(rootNode, std::filesystem::directory_iterator(rootPath));

	return rootNode;
}

void DirectoryTreeView::SetLastHoveredNode(const DirectoryTreeViewNode* node)
{
	lastHoveredNode = node;
	isHoveringNodeThisFrame = true;
}

void DirectoryTreeView::RecursivelyDisplayDirectoryNode(const DirectoryTreeViewNode& parentNode)
{
	ImGui::PushID(&parentNode);
	if (parentNode.isDirectory)
	{
		if (ImGui::TreeNodeEx(parentNode.fileName.c_str(), ImGuiTreeNodeFlags_SpanFullWidth))
		{
			if (ImGui::IsItemHovered())
				SetLastHoveredNode(&parentNode);
			for (const DirectoryTreeViewNode& childNode : parentNode.children)
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
		if (ImGui::TreeNodeEx(parentNode.fileName.c_str(), ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_SpanFullWidth))
		{
			if (ImGui::IsItemClicked(0))
			{
				if (fileClickCallback != nullptr)
					fileClickCallback(parentNode.fullPath);
			}
		}
		if (ImGui::IsItemHovered())
			SetLastHoveredNode(&parentNode);
	}
	ImGui::PopID();
}
