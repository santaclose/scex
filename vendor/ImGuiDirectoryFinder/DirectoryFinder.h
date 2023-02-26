#pragma once

#include <imgui.h>
#include <string>
#include <vector>
#include <thread>

#define INPUT_BUFFER_SIZE 256

struct DirectoryFinderSearchResult
{
	std::string filePath;
	int line;
	int startCharIndex;
	int endCharIndex;
};

struct DirectoryFinder
{
	typedef void (*OnResultClickCallback)(const DirectoryFinderSearchResult& result);

	std::string panelName = "Folder search";
	std::string directoryPath;
	bool regexEnabled = true;
	bool caseSensitiveEnabled = true;
	char toFind[INPUT_BUFFER_SIZE];
	char toInclude[INPUT_BUFFER_SIZE];
	char toExclude[INPUT_BUFFER_SIZE];

	std::vector<DirectoryFinderSearchResult> results;

	DirectoryFinder(const std::string& folderPath, OnResultClickCallback onResultClickCallback = nullptr);
	bool OnImGui();


private:
	OnResultClickCallback onResultClickCallback = nullptr;
	std::thread* finderThread = nullptr;

	void Find();
};