#include "FileTextEdit.h"

#include <filesystem>
#include <fstream>
#include <portable-file-dialogs.h>

std::unordered_map<std::string, const TextEditor::LanguageDefinition*> FileTextEdit::extensionToLanguageDefinition = {
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
	{".lua",&TextEditor::LanguageDefinition::Lua()},
	{".cs",&TextEditor::LanguageDefinition::CSharp()},
	{".json",&TextEditor::LanguageDefinition::Json()}
};

FileTextEdit::FileTextEdit(const char* filePath, int id, int createdFromFolderView, OnFindFileKeyComboCallback onFindFileKeyComboCallback)
{
	this->id = id;
	this->createdFromFolderView = createdFromFolderView;
	this->onFindFileKeyComboCallback = onFindFileKeyComboCallback;
	editor = new TextEditor();
	if (filePath == nullptr)
		panelName = "untitled##" + std::to_string((int)this);
	else
	{
		hasAssociatedFile = true;
		associatedFile = std::string(filePath);
		auto pathObject = std::filesystem::path(filePath);
		panelName = pathObject.filename().string() + "##" + std::to_string((int)this);
		std::ifstream t(filePath);
		std::string str((std::istreambuf_iterator<char>(t)),
			std::istreambuf_iterator<char>());
		editor->SetText(str);
		auto lang = extensionToLanguageDefinition.find(pathObject.extension().string());
		if (lang != extensionToLanguageDefinition.end())
			editor->SetLanguageDefinition(*(*lang).second);
	}
}

FileTextEdit::~FileTextEdit()
{
	delete editor;
}

bool FileTextEdit::OnImGui()
{
	bool windowIsOpen = true;
	if (showDebugPanel)
		editor->ImGuiDebugPanel("Debug " + panelName);

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0.0f, 0.0f });
	ImGui::Begin(panelName.c_str(), &windowIsOpen,
		ImGuiWindowFlags_MenuBar |
		ImGuiWindowFlags_NoSavedSettings |
		(editor->GetUndoIndex() != undoIndexInDisk ? ImGuiWindowFlags_UnsavedDocument : 0x0));
	ImGui::PopStyleVar();

	bool isFocused = ImGui::IsWindowFocused();
	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (hasAssociatedFile && ImGui::MenuItem("Reload", "Ctrl+R"))
				OnReloadCommand();
			if (ImGui::MenuItem("Load from"))
				OnLoadFromCommand();
			if (ImGui::MenuItem("Save", "Ctrl+S"))
				OnSaveCommand();
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
				editor->SelectAll();

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
			ImGui::SliderInt("Tab size", &tabSize, 1, 8);
			editor->SetTabSize(tabSize);
			ImGui::EndMenu();
		}

		auto cpos = editor->GetCursorPosition();
		ImGui::Text("%6d/%-6d %6d lines | %s | %s", cpos.mLine + 1, cpos.mColumn + 1, editor->GetTotalLines(),
			editor->IsOverwrite() ? "Ovr" : "Ins",
			editor->GetLanguageDefinitionName());

		ImGui::EndMenuBar();
	}

	isFocused |= editor->Render("TextEditor", isFocused);
	if (isFocused)
	{
		bool ctrlPressed = ImGui::GetIO().KeyCtrl;
		if (ctrlPressed && ImGui::IsKeyDown(ImGuiKey_S))
			OnSaveCommand();
		if (ctrlPressed && ImGui::IsKeyDown(ImGuiKey_R))
			OnReloadCommand();
		if (onFindFileKeyComboCallback != nullptr && ctrlPressed && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_P), false))
			onFindFileKeyComboCallback(createdFromFolderView);
	}

	ImGui::End();

	return windowIsOpen;
}

void FileTextEdit::SetSelection(int startLine, int startChar, int endLine, int endChar)
{
	editor->SetCursorPosition(endLine, endChar);
	editor->SetSelection(startLine, startChar, endLine, endChar);
}

const char* FileTextEdit::GetAssociatedFile()
{
	if (!hasAssociatedFile)
		return nullptr;
	return associatedFile.c_str();
}

void FileTextEdit::SetShowDebugPanel(bool value)
{
	showDebugPanel = value;
}

// Commands

void FileTextEdit::OnReloadCommand()
{
	std::ifstream t(associatedFile);
	std::string str((std::istreambuf_iterator<char>(t)),
		std::istreambuf_iterator<char>());
	editor->SetText(str);
	undoIndexInDisk = 0;
}

void FileTextEdit::OnLoadFromCommand()
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
	undoIndexInDisk = -1; // assume they are loading text from some other file
}

void FileTextEdit::OnSaveCommand()
{
	std::string textToSave = editor->GetText();
	std::string destination = hasAssociatedFile ?
		associatedFile :
		pfd::save_file("Save file", "", { "Any file", "*" }).result();
	if (destination.length() > 0)
	{
		associatedFile = destination;
		hasAssociatedFile = true;
		panelName = std::filesystem::path(destination).filename().string() + "##" + std::to_string((int)this);
		std::ofstream outFile;
		outFile.open(destination);
		outFile << textToSave;
		outFile.close();
	}
	undoIndexInDisk = editor->GetUndoIndex();
}