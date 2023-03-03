#include "DirectoryFinder.h"

#include <filesystem>
#include <fstream>
#include <regex>

DirectoryFinder::DirectoryFinder(const std::string& folderPath, OnResultClickCallback onResultClickCallback, int id, int createdFromFolderView)
{
	this->id = id;
	this->createdFromFolderView = createdFromFolderView;
	this->onResultClickCallback = onResultClickCallback;
	panelName = "Folder search##" + std::to_string((int)this);
	directoryPath = folderPath;
	toInclude[0] = toExclude[0] = toFind[0] = '\0';
}

bool DirectoryFinder::OnImGui()
{
	bool windowIsOpen = true;
	if (ImGui::Begin(panelName.c_str(), &windowIsOpen, ImGuiWindowFlags_NoSavedSettings))
	{
		ImGui::Text(directoryPath.c_str());
		ImGui::Checkbox(".*", &regexEnabled);
		ImGui::Checkbox("Aa", &caseSensitiveEnabled);
		ImGui::InputText("To find", toFind, INPUT_BUFFER_SIZE);
		ImGui::InputText("To include", toInclude, INPUT_BUFFER_SIZE);
		ImGui::InputText("To exclude", toExclude, INPUT_BUFFER_SIZE);
		if (finderThread == nullptr)
		{
			if (ImGui::Button("Find"))
				finderThread = new std::thread(&DirectoryFinder::Find, this);
		}
		else
		{
			if (ImGui::Button("Cancel search"))
				finderThread = nullptr;
		}

		{
			std::lock_guard<std::mutex> guard(finderThreadMutex);
			for (int i = 0; i < resultFiles.size(); i++)
			{
				DirectoryFinderSearchResultFile& file = resultFiles[i];
				ImGui::Separator();
				ImGui::TextUnformatted(file.fileName.c_str());
				for (int j = 0; j < file.results.size(); j++)
				{
					DirectoryFinderSearchResult& res = file.results[j];
					if (ImGui::Selectable(res.displayText.c_str()) && onResultClickCallback != nullptr)
						onResultClickCallback(file.filePath, res, createdFromFolderView);
				}
			}
		}
	}
	ImGui::End();
	return windowIsOpen;
}

void DirectoryFinder::Find()
{
	{
		std::lock_guard<std::mutex> guard(finderThreadMutex);
		resultFiles.clear();
	}

	std::string toFindAsStdString = std::string(toFind);
	std::regex toIncludeAsPattern = std::regex(toInclude);
	std::regex toExcludeAsPattern = std::regex(toExclude);
	std::regex toFindAsPattern = caseSensitiveEnabled ? std::regex(toFind) : std::regex(toFind, std::regex_constants::icase);

	bool foundInFile = false;
	for (std::filesystem::recursive_directory_iterator i(directoryPath), end; i != end; ++i)
	{
		if (finderThread == nullptr)
			break;
		if (is_directory(i->path()))
			continue;

		foundInFile = false;

		std::string filePath = i->path().string();
		std::string fileName = i->path().filename().string();
		std::smatch filePathMatch;
		if (toInclude[0] != '\0' && !std::regex_match(filePath, filePathMatch, toIncludeAsPattern))
			continue;
		if (toExclude[0] != '\0' && std::regex_match(filePath, filePathMatch, toExcludeAsPattern))
			continue;

		std::ifstream fileInput;
		fileInput.open(filePath.c_str());
		std::string line;
		int curLine = 0;
		while (std::getline(fileInput, line))
		{
			if (finderThread == nullptr)
				break;
			curLine++;
			if (!regexEnabled)
			{
				if (caseSensitiveEnabled)
				{
					int startChar = line.find(toFind, 0);
					if (startChar != std::string::npos)
					{
						std::lock_guard<std::mutex> guard(finderThreadMutex);
						if (!foundInFile)
						{
							resultFiles.push_back({ filePath, fileName, {} });
							foundInFile = true;
						}
						resultFiles.back().results.push_back({ std::to_string(curLine) + ": " + line, curLine, startChar, startChar + (int)toFindAsStdString.length()});
					}
				}
				else // caseSensitiveEnabled
				{
					auto it = std::search(
						line.begin(), line.end(),
						toFindAsStdString.begin(), toFindAsStdString.end(),
						[](unsigned char ch1, unsigned char ch2) { return std::toupper(ch1) == std::toupper(ch2); }
					);
					if (it != line.end())
					{
						std::lock_guard<std::mutex> guard(finderThreadMutex);
						int startChar = it - line.begin();
						if (!foundInFile)
						{
							resultFiles.push_back({ filePath, fileName, {} });
							foundInFile = true;
						}
						resultFiles.back().results.push_back({ std::to_string(curLine) + ": " + line, curLine, startChar, startChar + (int)toFindAsStdString.length() });
					}
				}
			}
			else // regexEnabled
			{
				std::smatch lineMatch;
				if (std::regex_search(line, lineMatch, toFindAsPattern))
				{
					std::lock_guard<std::mutex> guard(finderThreadMutex);
					if (!foundInFile)
					{
						resultFiles.push_back({ filePath, fileName, {} });
						foundInFile = true;
					}
					resultFiles.back().results.push_back({ std::to_string(curLine) + ": " + line, curLine, (int)lineMatch.position(), (int)(lineMatch.position() + lineMatch.length()) });
				}
			}
		}
		fileInput.close();
	}

	finderThread = nullptr;
}