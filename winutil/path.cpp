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

wstring& path::trim_backslash(wstring& path)
{
	while (path.back() == L'\\') {
		path.resize(path.size() - 1);
	}
	return path;
}

bool path::is_same(const wstring& path, const wstring& other)
{
	return str::eqi(path, other);
}

bool path::has_extension(const wstring& path, const wchar_t* extension)
{
	return str::ends_withi(path, extension);
}

bool path::has_extension(const wstring& path, initializer_list<const wchar_t*> extensions)
{
	for (const wchar_t* ext : extensions) {
		if (path::has_extension(path, ext)) {
			return true;
		}
	}
	return false;
}

wstring& path::change_extension(wstring& path, const wchar_t* newExtension)
{
	size_t dotIdx = path.find_last_of(L'.');
	if (dotIdx != wstring::npos) {
		path.resize(dotIdx + 1); // truncate after the dot
	} else {
		path.append(1, L'.');
	}
	path.append(newExtension[0] == L'.' ? newExtension + 1 : newExtension);
	return path;
}

wstring path::folder_from(const wstring& path)
{
	wstring ret(path);
	ret.resize(ret.find_last_of(L'\\')); // also remove trailing backslash
	return ret;
}

wstring path::file_from(const wstring& path)
{
	wstring ret(path);
	ret.erase(0, ret.find_last_of(L'\\') + 1);
	return ret;
}