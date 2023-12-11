#include "FontManager.h"

#include <string>
#include <iostream>
#include <unordered_map>

#include <PathUtils.h>

#define MIN_CODE_FONT_SIZE 7
#define MAX_CODE_FONT_SIZE 40

namespace FontManager {
	std::string uiFontPath;
	std::string codeFontPath;

	int defaultUiFontSize;

	ImFont* uiFont;
	std::unordered_map<int, ImFont*> codeFonts;
}

void FontManager::Initialize(ImGuiIO& io, int fontSize)
{
	defaultUiFontSize = fontSize;
	codeFontPath = PathUtils::GetAssetsDirectory() + "fonts/FiraCode/FiraCode-Regular.ttf";
	uiFontPath = PathUtils::GetAssetsDirectory() + "fonts/Inter/Inter.ttc";

	uiFont = io.Fonts->AddFontFromFileTTF(uiFontPath.c_str(), fontSize);
	for (int i = MIN_CODE_FONT_SIZE; i <= MAX_CODE_FONT_SIZE; i++)
		codeFonts[i] = io.Fonts->AddFontFromFileTTF(codeFontPath.c_str(), i);
}


ImFont* FontManager::GetUiFont()
{
	return uiFont;
}

ImFont* FontManager::GetCodeFont(int desiredSize)
{
	if (desiredSize > MAX_CODE_FONT_SIZE)
	{
		std::cout << "[FontManager] Clamping font size. Desired font size greater than max: " << desiredSize << " > " << MAX_CODE_FONT_SIZE << std::endl;
		desiredSize = MAX_CODE_FONT_SIZE;
	}
	else if (desiredSize < MIN_CODE_FONT_SIZE)
	{
		std::cout << "[FontManager] Clamping font size. Desired font size smaller than min: " << desiredSize << " < " << MIN_CODE_FONT_SIZE << std::endl;
		desiredSize = MIN_CODE_FONT_SIZE;
	}
	return codeFonts[desiredSize];
}

int FontManager::GetDefaultUiFontSize()
{
	return defaultUiFontSize;
}

int FontManager::GetMaxCodeFontSize()
{
	return MAX_CODE_FONT_SIZE;
}

int FontManager::GetMinCodeFontSize()
{
	return MIN_CODE_FONT_SIZE;
}
