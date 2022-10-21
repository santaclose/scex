#include "ImGuiController.h"

#include <string>
#include <iostream>
#include <fstream>
#include <filesystem>

#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <imgui.h>

#include <TextEditor.h>
#include <json.hpp>
#include <portable-file-dialogs.h>
#include <DirectoryTreeView.h>

namespace ste::ImGuiController
{
	bool menuBarEnabled = true;
	bool statsEnabled = false;
	int editorIdCounter = 0;
	int folderViewerIdCounter = 0;

	std::unordered_map<std::string, const TextEditor::LanguageDefinition*> extensionToLanguageDefinition = {
		{".cpp", &TextEditor::LanguageDefinition::CPlusPlus()},
		{".hpp", &TextEditor::LanguageDefinition::CPlusPlus()},
		{".cc", &TextEditor::LanguageDefinition::CPlusPlus()},
		{".hlsl", &TextEditor::LanguageDefinition::HLSL()},
		{".glsl", &TextEditor::LanguageDefinition::GLSL()},
		{".py", &TextEditor::LanguageDefinition::Python()},
		{".c", &TextEditor::LanguageDefinition::C()},
		{".h", &TextEditor::LanguageDefinition::C()},
		{".sql", &TextEditor::LanguageDefinition::SQL()},
		{".as", &TextEditor::LanguageDefinition::AngelScript()},
		{".lua",& TextEditor::LanguageDefinition::Lua()}
	};

	struct TextEditorInfo
	{
		int id;
		bool hasAssociatedFile = false;
		std::string panelName;
		std::string associatedFile;
		bool panelIsOpen = false;
	};

	struct FolderViewerInfo
	{
		int id;
		std::string panelName;
		std::string folderPath;
	};

	std::unordered_map<std::string, TextEditor*> fileToEditorMap;
	TextEditor* editorToFocus = nullptr;

	std::unordered_map<TextEditor*, TextEditorInfo> textEditors;
	std::vector<FolderViewerInfo> folderViewers;

	void CreateNewEditor(const std::string* filePath = nullptr)
	{
		TextEditor* editor = new TextEditor();
		if (filePath == nullptr)
			textEditors.insert({ editor, {editorIdCounter, false, "untitled##" + std::to_string(editorIdCounter), "", true} });
		else
		{
			auto pathObject = std::filesystem::path(*filePath);
			textEditors.insert({ editor, { editorIdCounter, true, pathObject.filename().string(), *filePath, true} });
			textEditors[editor].panelName += "##" + std::to_string(editorIdCounter);
			fileToEditorMap[*filePath] = editor;
			std::ifstream t(*filePath);
			std::string str((std::istreambuf_iterator<char>(t)),
				std::istreambuf_iterator<char>());
			editor->SetText(str);
			auto lang = extensionToLanguageDefinition.find(pathObject.extension().string());
			if (lang != extensionToLanguageDefinition.end())
				editor->SetLanguageDefinition(*(*lang).second);
		}
		editorIdCounter++;
	}

	void CreateNewFolderViewer(const std::string& folderPath)
	{
		folderViewers.push_back({ folderViewerIdCounter, "Folder view##" + std::to_string(folderViewerIdCounter), folderPath});
		folderViewerIdCounter++;
	}

	bool EditorTick(TextEditor* editor)
	{
		auto cpos = editor->GetCursorPosition();
		if (editorToFocus == editor)
		{
			ImGui::SetNextWindowFocus();
			editorToFocus = nullptr;
		}
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0.0f, 0.0f });
		ImGui::Begin(textEditors[editor].panelName.c_str(), &(textEditors[editor].panelIsOpen),
			ImGuiWindowFlags_HorizontalScrollbar |
			ImGuiWindowFlags_MenuBar |
			ImGuiWindowFlags_NoSavedSettings |
			(editor->CanUndo() ? ImGuiWindowFlags_UnsavedDocument : 0x0));
		ImGui::PopStyleVar();

		bool isFocused = ImGui::IsWindowFocused();
		ImGui::SetWindowSize(ImVec2(800, 600), ImGuiCond_FirstUseEver);
		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (textEditors[editor].hasAssociatedFile && ImGui::MenuItem("Reload", "Ctrl+R"))
				{
					std::ifstream t(textEditors[editor].associatedFile);
					std::string str((std::istreambuf_iterator<char>(t)),
						std::istreambuf_iterator<char>());
					editor->SetText(str);
				}
				if (ImGui::MenuItem("Load from"))
				{
					std::vector<std::string> selection = pfd::open_file("Open file", "", { "Any file", "*" }).result();
					if (selection.size() == 0)
						std::cout << "File not loaded\n";
					else
					{
						std::ifstream t(selection[0]);
						std::string str((std::istreambuf_iterator<char>(t)),
							std::istreambuf_iterator<char>());
						editor->SetText(str);
						auto pathObject = std::filesystem::path(selection[0]);
						auto lang = extensionToLanguageDefinition.find(pathObject.extension().string());
						if (lang != extensionToLanguageDefinition.end())
							editor->SetLanguageDefinition(*extensionToLanguageDefinition[pathObject.extension().string()]);
					}
				}
				if (ImGui::MenuItem("Save", "Ctrl+S"))
				{
					std::string textToSave = editor->GetText();
					std::string destination = textEditors[editor].hasAssociatedFile ?
						textEditors[editor].associatedFile :
						pfd::save_file("Save file", "", { "Any file", "*" }).result();
					if (destination.length() > 0)
					{
						textEditors[editor].associatedFile = destination;
						textEditors[editor].hasAssociatedFile = true;
						textEditors[editor].panelName = std::filesystem::path(destination).filename().string() + "##" + std::to_string(textEditors[editor].id);
						std::ofstream outFile;
						outFile.open(destination);
						outFile << textToSave;
						outFile.close();
					}
				}
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Edit"))
			{
				bool ro = editor->IsReadOnly();
				if (ImGui::MenuItem("Read-only mode", nullptr, &ro))
					editor->SetReadOnly(ro);
				ImGui::Separator();

				if (ImGui::MenuItem("Undo", "ALT-Backspace", nullptr, !ro && editor->CanUndo()))
					editor->Undo();
				if (ImGui::MenuItem("Redo", "Ctrl+Y", nullptr, !ro && editor->CanRedo()))
					editor->Redo();

				ImGui::Separator();

				if (ImGui::MenuItem("Copy", "Ctrl+C", nullptr, editor->HasSelection()))
					editor->Copy();
				if (ImGui::MenuItem("Cut", "Ctrl+X", nullptr, !ro && editor->HasSelection()))
					editor->Cut();
				if (ImGui::MenuItem("Delete", "Del", nullptr, !ro && editor->HasSelection()))
					editor->Delete();
				if (ImGui::MenuItem("Paste", "Ctrl+V", nullptr, !ro && ImGui::GetClipboardText() != nullptr))
					editor->Paste();

				ImGui::Separator();

				if (ImGui::MenuItem("Select all", nullptr, nullptr))
					editor->SetSelection(TextEditor::Coordinates(), TextEditor::Coordinates(editor->GetTotalLines(), 0));

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("View"))
			{
				if (ImGui::MenuItem("Mariana palette"))
					editor->SetPalette(TextEditor::GetMarianaPalette());
				if (ImGui::MenuItem("Dark palette"))
					editor->SetPalette(TextEditor::GetDarkPalette());
				if (ImGui::MenuItem("Light palette"))
					editor->SetPalette(TextEditor::GetLightPalette());
				if (ImGui::MenuItem("Retro blue palette"))
					editor->SetPalette(TextEditor::GetRetroBluePalette());
				ImGui::EndMenu();
			}
			ImGui::EndMenuBar();
		}

		ImGui::Text("%6d/%-6d %6d lines  | %s | %s | %s", cpos.mLine + 1, cpos.mColumn + 1, editor->GetTotalLines(),
			editor->IsOverwrite() ? "Ovr" : "Ins",
			editor->CanUndo() ? "*" : " ",
			editor->GetLanguageDefinitionName());
		editor->Render("TextEditor", isFocused);
		ImGui::End();
		return textEditors[editor].panelIsOpen;
	}

	// ---- Callbacks from folder view ---- //
	void OnFileClickedInFolderView(const std::string& filePath)
	{
		if (fileToEditorMap.find(filePath) == fileToEditorMap.end() || fileToEditorMap[filePath] == nullptr)
			CreateNewEditor(&filePath);
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
	auto f = io.Fonts->AddFontFromFileTTF(fontPath.c_str(), fontSize);

	DirectoryTreeView::SetOnFileClickCallback(OnFileClickedInFolderView);
	DirectoryTreeView::AddFileContextMenuOption("Show in folder", OnFileShowInFolder);
	DirectoryTreeView::AddFolderContextMenuOption("Show", OnFolderShow);
}

bool ste::ImGuiController::HasControl()
{
	return ImGui::GetIO().WantCaptureMouse;
}

void ste::ImGuiController::Tick(float deltaTime)
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
						CreateNewEditor(&(selection[0]));
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
				ImGui::MenuItem("Stats", NULL, &statsEnabled);
				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		}
	}

	if (statsEnabled)
	{
		ImGui::Begin("Stats");
		ImGui::Text("Frame time: %.3f ms", 1000.0f * deltaTime);
		ImGui::Text("FPS: %.1f", 1.0f / deltaTime);
		ImGui::End();
	}

	int i = 0, folderViewerToDelete = -1;
	for (auto& folderViewer : folderViewers)
	{
		if (!DirectoryTreeView::OnImGui(folderViewer.folderPath, folderViewer.panelName))
			folderViewerToDelete = i;
		i++;
	}
	if (folderViewerToDelete > -1)
		folderViewers.erase(folderViewers.begin() + folderViewerToDelete);

	TextEditor* editorToDelete = nullptr;
	for (auto editor : textEditors)
	{
		if (editor.first != nullptr)
		{
			if (!EditorTick(editor.first))
				editorToDelete = editor.first;
		}
	}
	if (editorToDelete != nullptr)
	{
		fileToEditorMap[textEditors[editorToDelete].associatedFile] = nullptr;
		delete editorToDelete;
		textEditors.erase(editorToDelete);
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
