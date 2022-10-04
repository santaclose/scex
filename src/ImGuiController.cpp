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

namespace ste::ImGuiController
{
	bool menuBarEnabled = true;
	bool statsEnabled = false;
	int editorIdCounter = 0;

	std::unordered_map<std::string, const TextEditor::LanguageDefinition*> extensionToLanguageDefinition = {
		{".cpp", &TextEditor::LanguageDefinition::CPlusPlus()},
		{".hpp", &TextEditor::LanguageDefinition::CPlusPlus()},
		{".cc", &TextEditor::LanguageDefinition::CPlusPlus()},
		{".hlsl", &TextEditor::LanguageDefinition::HLSL()},
		{".glsl", &TextEditor::LanguageDefinition::GLSL()},
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
	};

	std::unordered_map<TextEditor*, TextEditorInfo> textEditors;

	void CreateNewEditor(TextEditorInfo* info = nullptr)
	{
		TextEditor* editor = new TextEditor();
		if (info == nullptr)
			textEditors.insert({ editor, {editorIdCounter, false, "untitled##" + std::to_string(editorIdCounter), ""} });
		else
		{
			textEditors.insert({ editor, *info });
			textEditors[editor].id = editorIdCounter;
			textEditors[editor].panelName += "##" + std::to_string(editorIdCounter);
			std::ifstream t(info->associatedFile);
			std::string str((std::istreambuf_iterator<char>(t)),
				std::istreambuf_iterator<char>());
			editor->SetText(str);
			auto lang = extensionToLanguageDefinition.find(std::filesystem::path(info->associatedFile).extension().string());
			if (lang != extensionToLanguageDefinition.end())
				editor->SetLanguageDefinition(*(*lang).second);
		}
		editorIdCounter++;
	}

	bool EditorTick(TextEditor* editor)
	{
		bool closingEditor = false;
		auto cpos = editor->GetCursorPosition();
		ImGui::PushID(editor);
		ImGui::Begin(textEditors[editor].panelName.c_str(), nullptr, ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_MenuBar);
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
				if (ImGui::MenuItem("Close", "Ctrl+W"))
				{
					closingEditor = true;
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
			editor->GetLanguageDefinition().mName.c_str());
		editor->Render("TextEditor", isFocused);
		ImGui::End();
		ImGui::PopID();
		return closingEditor;
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
	//io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows

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
				{
					std::cout << "creating new panel\n";
					CreateNewEditor();
				}
				if (ImGui::MenuItem("Open File", "Ctrl+O"))
				{
					std::cout << "opening file\n";
					std::vector<std::string> selection = pfd::open_file("Open file", "", { "Any file", "*" }).result();
					if (selection.size() == 0)
						std::cout << "File not loaded\n";
					else
					{
						TextEditorInfo editorInfo = { 0, true, std::filesystem::path(selection[0]).filename().string(), selection[0]};
						CreateNewEditor(&editorInfo);
					}
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

	TextEditor* editorToDelete = nullptr;
	for (auto editor : textEditors)
	{
		if (editor.first != nullptr)
		{
			if (EditorTick(editor.first))
				editorToDelete = editor.first;
		}
	}
	delete editorToDelete;
	textEditors.erase(editorToDelete);

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
