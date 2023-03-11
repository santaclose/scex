#include "DirectoryTreeView.h"

DirectoryTreeView::DirectoryTreeView(
	const std::string& folderPath,
	OnFileClickCallback fileClickCallback,
	std::vector<std::pair<std::string, OnContextMenuCallback>>* fileContextMenuOptions,
	std::vector<std::pair<std::string, OnContextMenuCallback>>* folderContextMenuOptions,
	int id)
{
	this->id = id;
	panelName = "Folder view##" + std::to_string((int)this);
	directoryPath = folderPath;
	this->fileClickCallback = fileClickCallback;
	this->fileContextMenuOptions = fileContextMenuOptions;
	this->folderContextMenuOptions = folderContextMenuOptions;
	findFilesBuffer[0] = '\0';
	directoryTreeRoot = CreateDirectoryNodeTreeFromPath(folderPath);
}

DirectoryTreeView::~DirectoryTreeView()
{
	for (auto& item : searchTrie.children)
		Trie::Free(item.second);
}

bool DirectoryTreeView::OnImGui()
{
	bool windowIsOpen = true;
	if (requestingFocus)
		ImGui::SetNextWindowFocus();
	if (ImGui::Begin(panelName.c_str(), &windowIsOpen, ImGuiWindowFlags_NoSavedSettings))
	{
		if (ImGui::IsWindowFocused() && ImGui::GetIO().KeyCtrl && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_P), false))
			RunSearch();

		if (searching)
		{
			if (ImGui::IsKeyDown(ImGuiKey_Escape))
				searching = false;

			if (requestingFocus)
			{
				ImGui::SetKeyboardFocusHere();
				requestingFocus = false;
			}
			if (ImGui::InputText("Find files", findFilesBuffer, FIND_FILES_BUFFER_SIZE)) // true when text changes
			{
				searchResults.clear();
				Trie::GetSuggestions(&searchTrie, std::string(findFilesBuffer), searchResults);
			}
			bool searchTextFieldHasFocus = ImGui::IsItemFocused();
			bool pressedEnterOnSearchbarAndThereAreSearchResults = searchTextFieldHasFocus && searchResults.size() > 0 && ImGui::IsKeyDown(ImGuiKey_Enter); // to be able to press enter right after typing without using arrow keys

			bool callbackCalled = false;
			for (const std::string& searchResult : searchResults)
			{
				if (callbackCalled)
					break;
				bool fileNameIsUnique = fileNameToPath[searchResult].size() == 1;
				for (const std::string& filePath : fileNameToPath[searchResult])
				{
					if (fileClickCallback != nullptr)
					{
						if ((fileNameIsUnique ? ImGui::Selectable(searchResult.c_str()) : ImGui::Selectable((searchResult + " (" + filePath + ")").c_str())) ||
							pressedEnterOnSearchbarAndThereAreSearchResults ||
							ImGui::IsItemFocused() && ImGui::IsKeyPressed(ImGuiKey_Enter))
						{
							fileClickCallback(filePath, id);
							ImGui::GetIO().ClearInputKeys();
							searching = false;
							callbackCalled = true;
							break;
						}
					}
				}
			}
			ImGui::Separator();
		}

		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 0.0f, 0.0f });
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 0.0f, 2.0f });
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

		ImGui::PopStyleVar(2);
	}
	ImGui::End();
	return windowIsOpen;
}

void DirectoryTreeView::RunSearch()
{
	requestingFocus = searching = true;
	findFilesBuffer[0] = '\0';
	searchResults.clear();
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
		else
		{
			std::string fileName = entry.path().filename().string();
			std::string filePath = entry.path().string();
			Trie::Insert(&searchTrie, fileName);
			fileNameToPath[fileName].push_back(filePath);
		}
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
		if (ImGui::TreeNodeEx(parentNode.fileName.c_str(), ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_FramePadding))
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
		if (ImGui::TreeNodeEx(parentNode.fileName.c_str(), ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_FramePadding))
		{
			if (ImGui::IsItemClicked(0))
			{
				if (fileClickCallback != nullptr)
					fileClickCallback(parentNode.fullPath, id);
			}
		}
		if (ImGui::IsItemHovered())
			SetLastHoveredNode(&parentNode);
	}
	ImGui::PopID();
}