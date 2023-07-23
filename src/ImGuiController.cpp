#include "ImGuiController.h"

#include <string>
#include <iostream>
#include <fstream>
#include <filesystem>

#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <imgui.h>
#include <imgui_internal.h>

#include <json.hpp>
#include <portable-file-dialogs.h>

#include <panels/DirectoryTreeView.h>
#include <panels/DirectoryFinder.h>
#include <panels/FileTextEdit.h>
#include <Utils.h>
#include <PathUtils.h>

#include <imgui_te_engine.h>
#include <imgui_te_ui.h>

namespace ste::ImGuiController
{
	int folderViewForLastFocusedPanel = -1;

	ImGuiID leftDockID = -1;
	ImGuiID rightDockID = -1;

	ImGuiTestEngine* engine;
	ImGuiTestEngineIO* test_io;

	// ---- Callback declarations ---- //
	void OnFolderShow(const std::string& folderPath, int folderViewId);
	void OnFindInFolder(const std::string& folderPath, int folderViewId);
	void OnFileShowInFolder(const std::string& filePath, int folderViewId);
	void OnFileClickedInFolderView(const std::string& filePath, int folderViewId);
	void OnFolderSearchResultClick(const std::string& filePath, const DirectoryFinderSearchResult& searchResult, int folderViewId);
	void OnFolderSearchResultFoundOrSearchFinished();
	void OnPanelFocused(int folderViewId);

	bool menuBarEnabled = true;
	bool textEditDebugInfo = false;
	bool testEngineUiEnabled = false;
	bool consoleEnabled = false;

	std::unordered_map<std::string, FileTextEdit*> fileToEditorMap;
	FileTextEdit* editorToFocus = nullptr;

	std::vector<DirectoryFinder*> folderFinders;
	std::vector<DirectoryTreeView*> folderViewers;
	std::vector<FileTextEdit*> fileTextEdits;

	std::vector<std::pair<std::string, DirectoryTreeView::OnContextMenuCallback>> folderViewFileContextMenuOptions = { {"Show in folder", OnFileShowInFolder} };
	std::vector<std::pair<std::string, DirectoryTreeView::OnContextMenuCallback>> folderViewFolderContextMenuOptions = { {"Show", OnFolderShow}, {"Find in folder", OnFindInFolder} };

	FileTextEdit* CreateNewEditor(const char* filePath = nullptr, int fromFolderView = -1)
	{
		int fileTextEditId = fileTextEdits.size();
		fileTextEdits.push_back(new FileTextEdit(filePath, fileTextEditId, fromFolderView, OnPanelFocused));
		fileTextEdits.back()->SetShowDebugPanel(textEditDebugInfo);
		return fileTextEdits.back();
	}

	void CreateNewFolderViewer(const std::string& folderPath)
	{
		int folderViewerId = folderViewers.size();
		folderViewers.push_back(new DirectoryTreeView(folderPath, OnFileClickedInFolderView, OnPanelFocused, &folderViewFileContextMenuOptions, &folderViewFolderContextMenuOptions, folderViewerId));
	}
	void CreateNewFolderSearch(const std::string& folderPath, int fromFolderView)
	{
		int folderSearchId = folderFinders.size();
		folderFinders.push_back(new DirectoryFinder(folderPath, OnFolderSearchResultClick, OnFolderSearchResultFoundOrSearchFinished, OnFolderSearchResultFoundOrSearchFinished, OnPanelFocused, folderSearchId, fromFolderView));
	}

	void InitializeLayout(ImGuiID dock_main_id)
	{
		leftDockID = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Left, 0.23f, NULL, &dock_main_id);
		rightDockID = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Right, 0.3f, NULL, &dock_main_id);
		ImGui::DockBuilderFinish(dock_main_id);
	}

	// ---- Callbacks from folder view ---- //
	void OnFolderShow(const std::string& folderPath, int folderViewId)
	{
		// doesn't work with non ASCII
		std::string command = "explorer \"" + folderPath + "\"";
		system(command.c_str());
	}
	void OnFindInFolder(const std::string& folderPath, int folderViewId)
	{
		CreateNewFolderSearch(folderPath, folderViewId);
	}
	void OnFileShowInFolder(const std::string& filePath, int folderViewId)
	{
		auto path = std::filesystem::path(filePath);
		std::string parentFolderPath = path.parent_path().u8string();
		// doesn't work with non ASCII
		std::string command = "explorer /select,\"" + path.u8string() + "\",\"" + parentFolderPath + "\"";
		system(command.c_str());
	}
	void OnFileClickedInFolderView(const std::string& filePath, int folderViewId)
	{
		if (fileToEditorMap.find(filePath) == fileToEditorMap.end() || fileToEditorMap[filePath] == nullptr)
			fileToEditorMap[filePath] = CreateNewEditor(filePath.c_str(), folderViewId);
		else
			editorToFocus = fileToEditorMap[filePath];
	}

	// ---- Callback from folder finder ---- //
	void OnFolderSearchResultClick(const std::string& filePath, const DirectoryFinderSearchResult& searchResult, int folderViewId)
	{
		FileTextEdit* targetEditor;
		if (fileToEditorMap.find(filePath) == fileToEditorMap.end() || fileToEditorMap[filePath] == nullptr)
			targetEditor = fileToEditorMap[filePath] = CreateNewEditor(filePath.c_str(), folderViewId);
		else
			targetEditor = editorToFocus = fileToEditorMap[filePath];
		targetEditor->SetSelection(searchResult.lineNumber - 1, searchResult.startCharIndex, searchResult.lineNumber - 1, searchResult.endCharIndex);
	}
	void OnFolderSearchResultFoundOrSearchFinished()
	{
		glfwPostEmptyEvent();
	}

	// ---- Generic Callbacks ---- //
	void OnPanelFocused(int folderViewId)
	{
		folderViewForLastFocusedPanel = folderViewId;
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

	std::string fontPath = PathUtils::GetAssetsDirectory() + "fonts/FiraCode/FiraCode-Regular.ttf";
	float fontSize = 17.0f;
	// Load from config file if exists
	std::ifstream i(PathUtils::GetAssetsDirectory() + "config.json");
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
	io.IniFilename = nullptr;

	// Initialize Test Engine
	engine = ImGuiTestEngine_CreateContext();
	test_io = &ImGuiTestEngine_GetIO(engine);
	test_io->ConfigVerboseLevel = ImGuiTestVerboseLevel_Info;
	test_io->ConfigVerboseLevelOnError = ImGuiTestVerboseLevel_Debug;

	// Register your Tests
	RegisterTestEngineTests(engine); // will call IM_REGISTER_TEST() etc.

	// Start test engine
	ImGuiTestEngine_Start(engine, ImGui::GetCurrentContext());

	// Optional: use default crash handler. You may use your own crash handler and call ImGuiTestEngine_CrashHandler() from it.
	ImGuiTestEngine_InstallDefaultCrashHandler();
}

void ste::ImGuiController::Setup(GLFWwindow* window, const std::string& fileToOpen)
{
	Setup(window);
	fileToEditorMap[fileToOpen] = CreateNewEditor(fileToOpen.c_str());
	menuBarEnabled = false;
}

bool ste::ImGuiController::HasControl()
{
	return ImGui::GetIO().WantCaptureMouse;
}

void ste::ImGuiController::Tick()
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	ImGuiID mainDockID = ImGui::DockSpaceOverViewport();
	if (leftDockID == -1)
		InitializeLayout(mainDockID);

	bool newTextPanelRequested = false;
	bool openFileRequested = false;
	bool openFolderRequested = false;
	bool fileSearchRequested = false;

	bool ctrlKeyDown = ImGui::GetIO().KeyCtrl;
	if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_F1)))
		menuBarEnabled = !menuBarEnabled;

	newTextPanelRequested |= ctrlKeyDown && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_N), false);
	openFileRequested |= ctrlKeyDown && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_O), false);
	fileSearchRequested |= ctrlKeyDown && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_P), false);

	if (menuBarEnabled)
	{
		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("ste"))
			{
				newTextPanelRequested |= ImGui::MenuItem("New text panel", "Ctrl+N");
				openFileRequested |= ImGui::MenuItem("Open file", "Ctrl+O");
				openFolderRequested |= ImGui::MenuItem("Open folder");
				ImGui::MenuItem("Menu bar visible", "F1", &menuBarEnabled);
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("debug"))
			{
				if (ImGui::MenuItem("Per panel info", NULL, &textEditDebugInfo))
				{
					for (FileTextEdit* fte : fileTextEdits)
					{
						if (fte == nullptr)
							continue;
						fte->SetShowDebugPanel(textEditDebugInfo);
					}
				}
				ImGui::MenuItem("Test engine panel", NULL, &testEngineUiEnabled);
				ImGui::MenuItem("Console enabled", NULL, &consoleEnabled);
				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		}
	}
	if (newTextPanelRequested)
		CreateNewEditor();
	else if (openFileRequested)
	{
		std::vector<std::string> selection = pfd::open_file("Open file", "", { "Any file", "*" }).result();
		if (selection.size() > 0) // if not canceled
			fileToEditorMap[selection[0]] = CreateNewEditor(selection[0].c_str());
	}
	else if (openFolderRequested)
	{
		std::string folder = pfd::select_folder("Open folder").result();
		if (folder.length() > 0) // if not canceled
			CreateNewFolderViewer(folder);
	}
	else if (fileSearchRequested)
	{
		if (folderViewForLastFocusedPanel >= 0 && folderViewers[folderViewForLastFocusedPanel] != nullptr)
			folderViewers[folderViewForLastFocusedPanel]->RunSearch();
	}

	{
		int folderViewToDelete = -1;
		for (int i = 0; i < folderViewers.size(); i++)
		{
			DirectoryTreeView* folderView = folderViewers[i];
			if (folderView == nullptr)
				continue;
			ImGui::SetNextWindowDockID(leftDockID, ImGuiCond_FirstUseEver);
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
			ImGui::SetNextWindowDockID(rightDockID, ImGuiCond_FirstUseEver);
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
			ImGui::SetNextWindowDockID(mainDockID, ImGuiCond_FirstUseEver);
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
			const char* associatedFile = fileTextEdits[fileTextEditToDelete]->GetAssociatedFile();
			if (associatedFile != nullptr)
				fileToEditorMap.erase(associatedFile);
			delete fileTextEdits[fileTextEditToDelete];
			fileTextEdits[fileTextEditToDelete] = nullptr;
		}
	}

	// Optionally: show test engine UI to browse/run test from the UI
	if (testEngineUiEnabled)
		ImGuiTestEngine_ShowTestEngineWindows(engine, NULL);
	if (consoleEnabled)
		ShowConsole();

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

	// Call after your rendering. This is mostly to support screen/video capturing features.
	ImGuiTestEngine_PostSwap(engine);
}
