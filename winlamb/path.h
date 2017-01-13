/**
 * Part of WinLamb - Win32 API Lambda Library
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winlamb
 */

#pragma once
#include "str.h"
#include <Shlobj.h>

namespace wl {

class path final {
protected:
	path() = default;

public:
	static std::wstring& trim_backslash(std::wstring& filePath) {
		while (filePath.back() == L'\\') {
			filePath.resize(filePath.size() - 1);
		}
		return filePath;
	}

	static bool is_same(const std::wstring& filePath, const std::wstring& other) {
		return !lstrcmpiW(filePath.c_str(), other.c_str());
	}

	static bool has_extension(const std::wstring& filePath, const wchar_t* extension) {
		if (extension[0] == L'.') {
			return str::ends_withi(filePath, extension);
		}

		wchar_t dotExtension[16] = { L'.', L'\0' };
		lstrcatW(dotExtension, extension);
		return str::ends_withi(filePath, dotExtension);
	}

	static bool has_extension(const std::wstring& filePath, const std::wstring& extension) {
		return has_extension(filePath, extension.c_str());
	}

	static bool has_extension(const std::wstring& filePath, std::initializer_list<const wchar_t*> extensions)
	{
		for (const wchar_t* ext : extensions) {
			if (has_extension(filePath, ext)) {
				return true;
			}
		}
		return false;
	}

	static std::wstring& change_extension(std::wstring& filePath, const wchar_t* newExtension)
	{
		size_t dotIdx = filePath.find_last_of(L'.');
		if (dotIdx != std::wstring::npos) {
			filePath.resize(dotIdx + 1); // truncate after the dot
		} else {
			filePath.append(1, L'.');
		}
		filePath.append(newExtension[0] == L'.' ? newExtension + 1 : newExtension);
		return filePath;
	}

	static std::wstring& change_extension(std::wstring& filePath, const std::wstring& newExtension) {
		return change_extension(filePath, newExtension.c_str());
	}

	static std::wstring folder_from(const std::wstring& filePath, bool withBackslash = false) {
		std::wstring ret(filePath);
		size_t found = ret.find_last_of(L'\\');
		if (found != std::wstring::npos) {
			ret.resize(found);
			if (withBackslash) ret += L'\\';
		}
		return ret;
	}

	static std::wstring file_from(const std::wstring& filePath) {
		std::wstring ret(filePath);
		size_t found = ret.find_last_of(L'\\');
		if (found != std::wstring::npos) {
			ret.erase(0, found + 1);
		}
		return ret;
	}

	static std::wstring exe_path() {
		wchar_t buf[MAX_PATH] = { L'\0' };
		GetModuleFileNameW(nullptr, buf, ARRAYSIZE(buf)); // full path name
		std::wstring ret = buf;
		ret.resize(ret.find_last_of(L'\\')); // truncate removing EXE filename and trailing backslash
#ifdef _DEBUG
		ret.resize(ret.find_last_of(L'\\')); // bypass "Debug" folder, remove trailing backslash too
#ifdef _WIN64
		ret.resize(ret.find_last_of(L'\\')); // bypass "x64" folder, remove trailing backslash again
#endif
#endif
		return ret;
	}

	static std::wstring desktop_path() {
		wchar_t buf[MAX_PATH] = { L'\0' };
		SHGetFolderPathW(nullptr, CSIDL_DESKTOPDIRECTORY, nullptr, 0, buf); // won't have trailing backslash
		return buf;
	}
};

}//namespace wl