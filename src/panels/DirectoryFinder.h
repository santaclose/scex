#pragma once

#include <imgui.h>
#include <string>
#include <vector>
#include <thread>
#include <mutex>

#define INPUT_BUFFER_SIZE 256

struct DirectoryFinderSearchResult
{
	std::string displayText;
	int lineNumber;
	int startCharIndex;
	int endCharIndex;
};

struct DirectoryFinderSearchResultFile
{
	std::string filePath;
	std::string fileName;
	std::vector<DirectoryFinderSearchResult> results;
};

struct DirectoryFinder
{
	typedef void (*OnResultClickCallback)(const std::string& filePath, const DirectoryFinderSearchResult& result, int originFolderView);

	DirectoryFinder(const std::string& folderPath, OnResultClickCallback onResultClickCallback = nullptr, int id = -1, int createdFromFolderView = -1);
	bool OnImGui();

private:
	int id = -1;
	int createdFromFolderView = -1;

	std::string panelName = "Folder search";
	std::string directoryPath;
	bool regexEnabled = true;
	bool caseSensitiveEnabled = true;
	char toFind[INPUT_BUFFER_SIZE];
	char toInclude[INPUT_BUFFER_SIZE];
	char toExclude[INPUT_BUFFER_SIZE];

	std::vector<DirectoryFinderSearchResultFile> resultFiles;

	OnResultClickCallback onResultClickCallback = nullptr;
	std::thread* finderThread = nullptr;
	std::mutex finderThreadMutex;

	void Find();
};