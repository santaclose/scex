#pragma once

#include <TextEditor.h>

struct FileTextEdit
{
	typedef void (*OnFindFileKeyComboCallback)(int linkedFolderView);

	FileTextEdit(const char* filePath = nullptr, int id = -1, int createdFromFolderView = -1, OnFindFileKeyComboCallback onFindFileKeyComboCallback = nullptr);
	~FileTextEdit();
	bool OnImGui();
	void SetSelection(int startLine, int startChar, int endLine, int endChar);
	const char* GetAssociatedFile();
	void SetShowDebugPanel(bool value);

private:
	int id = -1;
	int createdFromFolderView = -1;

	OnFindFileKeyComboCallback onFindFileKeyComboCallback = nullptr;

	TextEditor* editor = nullptr;
	bool showDebugPanel = false;
	bool hasAssociatedFile = false;
	std::string panelName;
	std::string associatedFile;
	int tabSize = 4;

	static std::unordered_map<std::string, const TextEditor::LanguageDefinition*> extensionToLanguageDefinition;
};