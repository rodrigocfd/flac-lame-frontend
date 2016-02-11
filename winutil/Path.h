
#pragma once
#include <string>

struct Path final {
	static std::wstring& trimBackslash(std::wstring& path);
	static bool          hasExtension(const std::wstring& path, std::initializer_list<const wchar_t*> extensions);
	static std::wstring& changeExtension(std::wstring& path, const wchar_t *newExtension);
	static std::wstring& changeExtension(std::wstring& path, const std::wstring& newExtension);
	static std::wstring  folderFrom(const std::wstring& path);
	static std::wstring  fileFrom(const std::wstring& path);
};