#pragma once
#include <optional>
#include <string_view>

namespace convert {

std::wstring iniPath();

void toWav(std::wstring_view iniPath, std::wstring_view srcFile, std::optional<std::wstring_view> destFolder, bool delSrc);
void toFlac(std::wstring_view iniPath, std::wstring_view srcFile, std::optional<std::wstring_view> destFolder, bool delSrc,
	std::wstring_view quality);
void toMp3(std::wstring_view iniPath, std::wstring_view srcFile, std::optional<std::wstring_view> destFolder, bool delSrc,
	std::wstring_view quality, bool isVbr);

}
