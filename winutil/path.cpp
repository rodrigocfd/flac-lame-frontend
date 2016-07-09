/**
 * Part of WinUtil - Windows Utilities
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winutil
 */

#include "path.h"
#include "str.h"
using namespace winutil;
using std::initializer_list;
using std::wstring;

wstring& path::trim_backslash(wstring& filePath)
{
	while (filePath.back() == L'\\') {
		filePath.resize(filePath.size() - 1);
	}
	return filePath;
}

bool path::is_same(const wstring& filePath, const wstring& other)
{
	return str::eqi(filePath, other);
}

bool path::has_extension(const wstring& filePath, const wchar_t* extension)
{
	if (extension[0] == L'.') {
		return str::ends_withi(filePath, extension);
	}

	wchar_t dotExtension[16] = { L'.', L'\0' };
	lstrcat(dotExtension, extension);
	return str::ends_withi(filePath, dotExtension);
}

bool path::has_extension(const wstring& filePath, initializer_list<const wchar_t*> extensions)
{
	for (const wchar_t* ext : extensions) {
		if (has_extension(filePath, ext)) {
			return true;
		}
	}
	return false;
}

wstring& path::change_extension(wstring& filePath, const wchar_t* newExtension)
{
	size_t dotIdx = filePath.find_last_of(L'.');
	if (dotIdx != wstring::npos) {
		filePath.resize(dotIdx + 1); // truncate after the dot
	} else {
		filePath.append(1, L'.');
	}
	filePath.append(newExtension[0] == L'.' ? newExtension + 1 : newExtension);
	return filePath;
}

wstring path::folder_from(const wstring& filePath)
{
	wstring ret(filePath);
	ret.resize(ret.find_last_of(L'\\')); // also remove trailing backslash
	return ret;
}

wstring path::file_from(const wstring& filePath)
{
	wstring ret(filePath);
	ret.erase(0, ret.find_last_of(L'\\') + 1);
	return ret;
}