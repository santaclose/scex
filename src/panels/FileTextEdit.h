#pragma once

#include <TextEditor.h>

#define FIND_POPUP_TEXT_FIELD_LENGTH 128

struct FileTextEdit
{
	typedef void (*OnFocusedCallback)(int folderViewId);
	typedef void (*OnShowInFolderViewCallback)(const std::string& filePath, int folderViewId);

	FileTextEdit(const char* filePath = nullptr,
		int id = -1,
		int createdFromFolderView = -1,
		OnFocusedCallback onFocusedCallback = nullptr,
		OnShowInFolderViewCallback onShowInFolderViewCallback = nullptr);
	~FileTextEdit();
	bool OnImGui();
	void SetSelection(int startLine, int startChar, int endLine, int endChar);
	const char* GetAssociatedFile();
	void OnFolderViewDeleted(int folderViewId);
	void SetShowDebugPanel(bool value);

private:

	// Commands
	void OnReloadCommand();
	void OnLoadFromCommand();
	void OnSaveCommand();

	int id = -1;
	int createdFromFolderView = -1;

	OnFocusedCallback onFocusedCallback = nullptr;
	OnShowInFolderViewCallback onShowInFolderViewCallback = nullptr;

	TextEditor* editor = nullptr;
	bool showDebugPanel = false;
	bool hasAssociatedFile = false;
	std::string panelName;
	std::string associatedFile;
	int tabSize = 4;
	float lineSpacing = 1.0f;
	int undoIndexInDisk = 0;

	char ctrlfTextToFind[FIND_POPUP_TEXT_FIELD_LENGTH] = "";
	bool ctrlfCaseSensitive = false;

	static std::unordered_map<std::string, const TextEditor::LanguageDefinitionId> extensionToLanguageDefinition;
};