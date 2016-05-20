/**
 * Part of WinUtil - Windows Utilities
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winutil
 */

#pragma once
#include <string>

namespace winutil {

struct path final {
	static std::wstring& trim_backslash(std::wstring& path);
	static bool          is_same(const std::wstring& path, const std::wstring& other);
	static bool          has_extension(const std::wstring& path, const wchar_t* extension);
	static bool          has_extension(const std::wstring& path, std::initializer_list<const wchar_t*> extensions);
	static std::wstring& change_extension(std::wstring& path, const wchar_t* newExtension);
	static std::wstring& change_extension(std::wstring& path, const std::wstring& newExtension) { return change_extension(path, newExtension.c_str()); }
	static std::wstring  folder_from(const std::wstring& path);
	static std::wstring  file_from(const std::wstring& path);
};

}//namespace winutil