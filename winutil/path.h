/**
 * Part of WinUtil - Windows Utilities
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winutil
 */

#pragma once
#include <string>

namespace winutil {

struct path final {
	static std::wstring& trim_backslash(std::wstring& filePath);
	static bool          is_same(const std::wstring& filePath, const std::wstring& other);
	static bool          has_extension(const std::wstring& filePath, const wchar_t* extension);
	static bool          has_extension(const std::wstring& filePath, const std::wstring& extension) { return has_extension(filePath, extension.c_str()); }
	static bool          has_extension(const std::wstring& filePath, std::initializer_list<const wchar_t*> extensions);
	static std::wstring& change_extension(std::wstring& filePath, const wchar_t* newExtension);
	static std::wstring& change_extension(std::wstring& filePath, const std::wstring& newExtension) { return change_extension(filePath, newExtension.c_str()); }
	static std::wstring  folder_from(const std::wstring& filePath);
	static std::wstring  file_from(const std::wstring& filePath);
};

}//namespace winutil