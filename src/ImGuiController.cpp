#include "ImGuiController.h"

#include <string>
#include <iostream>
#include <fstream>
#include <filesystem>

#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <imgui.h>

#include <json.hpp>
#include <portable-file-dialogs.h>

#include <panels/DirectoryTreeView.h>
#include <panels/DirectoryFinder.h>
#include <panels/FileTextEdit.h>

#define DEFAULT_TEXT_EDITOR_WIDTH 800
#define DEFAULT_TEXT_EDITOR_HEIGHT 600
#define DEFAULT_FOLDER_VIEW_WIDTH 250
#define DEFAULT_FOLDER_VIEW_HEIGHT 600

namespace ste::ImGuiController
{
	// ---- Callback declarations ---- //
	void OnFileClickedInFolderView(const std::string& filePath);
	void OnFileShowInFolder(const std::string& filePath);
	void OnFolderShow(const std::string& folderPath);
	void OnFindInFolder(const std::string& folderPath);
	void OnFolderSearchResultClick(const DirectoryFinderSearchResult& searchResult);

	bool menuBarEnabled = true;
	bool textEditDebugInfo = false;

	std::unordered_map<std::string, FileTextEdit*> fileToEditorMap;
	FileTextEdit* editorToFocus = nullptr;

	std::vector<DirectoryFinder*> folderFinders;
	std::vector<DirectoryTreeView*> folderViewers;
	std::vector<FileTextEdit*> fileTextEdits;

	std::vector<std::pair<std::string, DirectoryTreeView::OnContextMenuCallback>> folderViewFileContextMenuOptions = { {"Show in folder", OnFileShowInFolder} };
	std::vector<std::pair<std::string, DirectoryTreeView::OnContextMenuCallback>> folderViewFolderContextMenuOptions = { {"Show", OnFolderShow}, {"Find in folder", OnFindInFolder} };

	FileTextEdit* CreateNewEditor(const char* filePath = nullptr)
	{
		fileTextEdits.push_back(new FileTextEdit(filePath));
		return fileTextEdits.back();
	}

	void CreateNewFolderViewer(const std::string& folderPath)
	{
		folderViewers.push_back(new DirectoryTreeView(folderPath, OnFileClickedInFolderView, &folderViewFileContextMenuOptions, &folderViewFolderContextMenuOptions));
	}
	void CreateNewFolderSearch(const std::string& folderPath)
	{
		folderFinders.push_back(new DirectoryFinder(folderPath, OnFolderSearchResultClick));
	}

	// ---- Callbacks from folder view ---- //
	void OnFileClickedInFolderView(const std::string& filePath)
	{
		if (fileToEditorMap.find(filePath) == fileToEditorMap.end() || fileToEditorMap[filePath] == nullptr)
			fileToEditorMap[filePath] = CreateNewEditor(filePath.c_str());
		else
			editorToFocus = fileToEditorMap[filePath];
	}
	void OnFileShowInFolder(const std::string& filePath)
	{
		auto path = std::filesystem::path(filePath);
		std::string parentFolderPath = path.parent_path().string();
		std::string command = "explorer /select,\"" + path.string() + "\",\"" + parentFolderPath + "\"";
		system(command.c_str());
	}
	void OnFolderShow(const std::string& folderPath)
	{
		std::string command = "explorer \"" + folderPath + "\"";
		system(command.c_str());
	}
	void OnFindInFolder(const std::string& folderPath)
	{
		CreateNewFolderSearch(folderPath);
	}

	// ---- Callback from folder finder ---- //
	void OnFolderSearchResultClick(const DirectoryFinderSearchResult& searchResult)
	{
		FileTextEdit* targetEditor;
		if (fileToEditorMap.find(searchResult.filePath) == fileToEditorMap.end() || fileToEditorMap[searchResult.filePath] == nullptr)
			targetEditor = fileToEditorMap[searchResult.filePath] = CreateNewEditor(searchResult.filePath.c_str());
		else
			targetEditor = editorToFocus = fileToEditorMap[searchResult.filePath];
		targetEditor->SetSelection(searchResult.line - 1, searchResult.startCharIndex, searchResult.line - 1, searchResult.endCharIndex);
	}
}

void ste::ImGuiController::Setup(GLFWwindow* window)
{
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows

	// Setup Platform/Renderer bindings
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init();

	std::string fontPath = "assets/fonts/FiraCode/FiraCode-Regular.ttf";
	float fontSize = 17.0f;
	// Load from config file if exists
	std::ifstream i("assets/config.json");
	if (i.good())
	{
		nlohmann::json j;
		i >> j;
		if (j.find("fontPath") != j.end())
			fontPath = j["fontPath"];
		if (j.find("fontSize") != j.end())
			fontSize = j["fontSize"];
		std::cout << "Config file loaded\n";
	}
	else
		std::cout << "Config file not found, using default values\n";

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	io.Fonts->AddFontFromFileTTF(fontPath.c_str(), fontSize);
}

bool ste::ImGuiController::HasControl()
{
	return ImGui::GetIO().WantCaptureMouse;
}

void ste::ImGuiController::Tick()
{
	if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_F1)))
		menuBarEnabled = !menuBarEnabled;

	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	ImGui::DockSpaceOverViewport();

	if (menuBarEnabled)
	{
		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("ste"))
			{
				if (ImGui::MenuItem("New panel", "Ctrl+N"))
					CreateNewEditor();
				if (ImGui::MenuItem("Open file", "Ctrl+O"))
				{
					std::vector<std::string> selection = pfd::open_file("Open file", "", { "Any file", "*" }).result();
					if (selection.size() == 0)
						std::cout << "File not loaded\n";
					else
						fileToEditorMap[selection[0]] = CreateNewEditor(selection[0].c_str());
				}
				if (ImGui::MenuItem("Open folder"))
				{
					std::string folder = pfd::select_folder("Open folder").result();
					if (folder.length() == 0)
						std::cout << "folder selection canceled\n";
					else
						CreateNewFolderViewer(folder);
				}
				ImGui::MenuItem("Menu bar visible", "F1", &menuBarEnabled);
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("debug"))
			{
				ImGui::MenuItem("Per panel info", NULL, &textEditDebugInfo);
				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		}
	}

	{
		int folderViewToDelete = -1;
		for (int i = 0; i < folderViewers.size(); i++)
		{
			DirectoryTreeView* folderView = folderViewers[i];
			if (folderView == nullptr)
				continue;
			ImGui::SetNextWindowSize(ImVec2(DEFAULT_FOLDER_VIEW_WIDTH, DEFAULT_FOLDER_VIEW_HEIGHT), ImGuiCond_FirstUseEver);
			if (!folderView->OnImGui())
				folderViewToDelete = i;
		}
		if (folderViewToDelete > -1)
		{
			delete folderViewers[folderViewToDelete];
			folderViewers[folderViewToDelete] = nullptr;
		}
	}
	{
		int finderToDelete = -1;
		for (int i = 0; i < folderFinders.size(); i++)
		{
			DirectoryFinder* finder = folderFinders[i];
			if (finder == nullptr)
				continue;
			ImGui::SetNextWindowSize(ImVec2(DEFAULT_FOLDER_VIEW_WIDTH, DEFAULT_FOLDER_VIEW_HEIGHT), ImGuiCond_FirstUseEver);
			if (!finder->OnImGui())
				finderToDelete = i;
		}
		if (finderToDelete > -1)
		{
			delete folderFinders[finderToDelete];
			folderFinders[finderToDelete] = nullptr;
		}
	}
	{
		int fileTextEditToDelete = -1;
		for (int i = 0; i < fileTextEdits.size(); i++)
		{
			FileTextEdit* fte = fileTextEdits[i];
			if (fte == nullptr)
				continue;
			ImGui::SetNextWindowSize(ImVec2(DEFAULT_TEXT_EDITOR_WIDTH, DEFAULT_TEXT_EDITOR_HEIGHT), ImGuiCond_FirstUseEver);
			if (editorToFocus == fte)
			{
				ImGui::SetNextWindowFocus();
				editorToFocus = nullptr;
			}
			if (!fte->OnImGui())
				fileTextEditToDelete = i;
		}
		if (fileTextEditToDelete > -1)
		{
			if (fileTextEdits[fileTextEditToDelete]->hasAssociatedFile)
				fileToEditorMap.erase(fileTextEdits[fileTextEditToDelete]->associatedFile);
			delete fileTextEdits[fileTextEditToDelete];
			fileTextEdits[fileTextEditToDelete] = nullptr;
		}
	}

	// Render dear imgui into screen
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		GLFWwindow* backup_current_context = glfwGetCurrentContext();
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
		glfwMakeContextCurrent(backup_current_context);
	}
}
