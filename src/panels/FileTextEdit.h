#pragma once

#include <TextEditor.h>

struct FileTextEdit
{
	TextEditor* editor = nullptr;
	bool showDebugPanel = false;
	bool hasAssociatedFile = false;
	std::string panelName;
	std::string associatedFile;
	int tabSize = 4;

	FileTextEdit(const char* filePath = nullptr);
	~FileTextEdit();
	bool OnImGui();
	void SetSelection(int startLine, int startChar, int endLine, int endChar);

	static std::unordered_map<std::string, const TextEditor::LanguageDefinition*> extensionToLanguageDefinition;
};