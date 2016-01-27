
#pragma once
#include <string>
#include <vector>
#include <Windows.h>

struct Str final {
	static std::wstring  format(const wchar_t *format, ...);
	static std::wstring& trimNulls(std::wstring& s);
	static std::wstring& trim(std::wstring& s);
	static std::wstring& upper(std::wstring& s);
	static std::wstring& lower(std::wstring& s);
	static std::wstring& removeDiacritics(std::wstring& s);
	static std::wstring  folderFromPath(const std::wstring& path);
	static std::wstring  fileFromPath(const std::wstring& path);
	static size_t        find(const std::wstring& s, const wchar_t *what, size_t offset = 0);
	static size_t        find(const std::wstring& s, const std::wstring& what, size_t offset = 0);
	static size_t        findI(const std::wstring& s, const wchar_t *what, size_t offset = 0);
	static size_t        findI(const std::wstring& s, const std::wstring& what, size_t offset = 0);
	static size_t        findLast(const std::wstring& s, const wchar_t *what, size_t offset = std::wstring::npos);
	static size_t        findLast(const std::wstring& s, const std::wstring& what, size_t offset = std::wstring::npos);
	static size_t        findLastI(const std::wstring& s, const wchar_t *what, size_t offset = std::wstring::npos);
	static size_t        findLastI(const std::wstring& s, const std::wstring& what, size_t offset = std::wstring::npos);
	static std::wstring& replace(std::wstring& s, const wchar_t *what, const wchar_t *replacement);
	static std::wstring& replaceI(std::wstring& s, const wchar_t *what, const wchar_t *replacement);
	static bool          eq(const std::wstring& s, const wchar_t *what);
	static bool          eq(const std::wstring& s, const std::wstring& what);
	static bool          eqI(const std::wstring& s, const wchar_t *what);
	static bool          eqI(const std::wstring& s, const std::wstring& what);
	static bool          endsWith(const std::wstring& s, const wchar_t *what);
	static bool          endsWith(const std::wstring& s, const std::wstring& what);
	static bool          endsWithI(const std::wstring& s, const wchar_t *what);
	static bool          endsWithI(const std::wstring& s, const std::wstring& what);
	static bool          beginsWith(const std::wstring& s, const wchar_t *what);
	static bool          beginsWith(const std::wstring& s, const std::wstring& what);
	static bool          beginsWithI(const std::wstring& s, const wchar_t *what);
	static bool          beginsWithI(const std::wstring& s, const std::wstring& what);
	static bool          isInt(const std::wstring& s);
	static bool          isUint(const std::wstring& s);
	static bool          isHex(const std::wstring& s);
	static bool          isFloat(const std::wstring& s);
	static std::vector<std::wstring> explode(const std::wstring& s, const wchar_t *delimiter);
	static std::vector<std::wstring> explode(const std::wstring& s, const std::wstring& delimiter);
	static std::vector<std::wstring> explodeMultiZero(const wchar_t *s);
	static std::vector<std::wstring> explodeQuoted(const wchar_t *s);
	static std::wstring              parseAscii(const BYTE *data, size_t length);
	static std::wstring              parseAscii(const std::vector<BYTE>& data);
	static std::wstring              parseUtf8(const BYTE *data, size_t length);
	static std::wstring              parseUtf8(const std::vector<BYTE>& data);
	static std::vector<BYTE>         serializeUtf8(const std::wstring& s);
};