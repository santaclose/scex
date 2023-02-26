#include "DirectoryFinder.h"

#include <filesystem>
#include <fstream>
#include <regex>

DirectoryFinder::DirectoryFinder(const std::string& folderPath, OnResultClickCallback onResultClickCallback)
{
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

		ImGui::Separator();
		for (int i = 0; i < results.size(); i++)
		{
			if (ImGui::Selectable((results[i].filePath + ": " + std::to_string(results[i].line)).c_str()) && onResultClickCallback != nullptr)
				onResultClickCallback(results[i]);
		}
	}
	ImGui::End();
	return windowIsOpen;
}

void DirectoryFinder::Find()
{
	results.clear();

	std::string toFindAsStdString = std::string(toFind);
	std::regex toIncludeAsPattern = std::regex(toInclude);
	std::regex toExcludeAsPattern = std::regex(toExclude);
	std::regex toFindAsPattern = caseSensitiveEnabled ? std::regex(toFind) : std::regex(toFind, std::regex_constants::icase);

	for (std::filesystem::recursive_directory_iterator i(directoryPath), end; i != end; ++i)
	{
		if (finderThread == nullptr)
			break;
		if (is_directory(i->path()))
			continue;
		std::string filePath = i->path().string();
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
						results.push_back({ filePath, curLine, startChar, startChar + (int)toFindAsStdString.length() });
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
						int startChar = it - line.begin();
						results.push_back({ filePath, curLine, startChar, startChar + (int)toFindAsStdString.length() });
					}
				}
			}
			else // regexEnabled
			{
				std::smatch lineMatch;
				if (std::regex_search(line, lineMatch, toFindAsPattern))
					results.push_back({ filePath, curLine, (int)lineMatch.position(), (int)(lineMatch.position() + lineMatch.length()) });
			}
		}
		fileInput.close();
	}

	finderThread = nullptr;
}