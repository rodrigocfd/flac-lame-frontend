#pragma once
#include <windlg/lib.h>

namespace convert {

void toWav(const lib::Ini& ini, std::wstring_view srcFile, std::optional<std::wstring_view> destFolder, bool delSrc);
void toFlac(const lib::Ini& ini, std::wstring_view srcFile, std::optional<std::wstring_view> destFolder, bool delSrc,
	std::wstring_view quality);
void toMp3(const lib::Ini& ini, std::wstring_view srcFile, std::optional<std::wstring_view> destFolder, bool delSrc,
	std::wstring_view quality, bool isVbr);

}
