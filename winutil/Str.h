
#pragma once
#include <string>
#include <vector>
#include <Windows.h>

struct Str final {
	static std::wstring  format(const wchar_t *format, ...);
	static std::wstring& trimNulls(std::wstring& s);
	static std::wstring& trim(std::wstring& s);
	static std::wstring  upper(const std::wstring& s);
	static std::wstring  lower(const std::wstring& s);
	static std::wstring& removeDiacritics(std::wstring& s);
	static bool          eqI(const std::wstring& s, const wchar_t *what)      { return !lstrcmpi(s.c_str(), what); }
	static bool          eqI(const std::wstring& s, const std::wstring& what) { return eqI(s.c_str(), what.c_str()); }
	static bool          endsWith(const std::wstring& s, const wchar_t *what);
	static bool          endsWith(const std::wstring& s, const std::wstring& what)    { return endsWith(s, what.c_str()); }
	static bool          endsWithI(const std::wstring& s, const wchar_t *what);
	static bool          endsWithI(const std::wstring& s, const std::wstring& what)   { return endsWithI(s, what.c_str()); }
	static bool          beginsWith(const std::wstring& s, const wchar_t *what);
	static bool          beginsWith(const std::wstring& s, const std::wstring& what)  { return beginsWith(s, what.c_str()); }
	static bool          beginsWithI(const std::wstring& s, const wchar_t *what);
	static bool          beginsWithI(const std::wstring& s, const std::wstring& what) { return beginsWithI(s, what.c_str()); }
	static size_t        findI(const std::wstring& s, const wchar_t *what, size_t offset = 0);
	static size_t        findI(const std::wstring& s, const std::wstring& what, size_t offset = 0)  { return findI(s, what.c_str(), offset); }
	static size_t        rfindI(const std::wstring& s, const wchar_t *what, size_t offset = 0);
	static size_t        rfindI(const std::wstring& s, const std::wstring& what, size_t offset = 0) { return rfindI(s, what.c_str(), offset); }
	static std::wstring& replace(std::wstring& haystack, const wchar_t *needle, const wchar_t *replacement);
	static std::wstring& replaceI(std::wstring& haystack, const wchar_t *needle, const wchar_t *replacement);
	static bool          isInt(const std::wstring& s);
	static bool          isUint(const std::wstring& s);
	static bool          isHex(const std::wstring& s);
	static bool          isFloat(const std::wstring& s);
	static std::vector<std::wstring> explode(const std::wstring& s, const wchar_t *delimiter);
	static std::vector<std::wstring> explode(const std::wstring& s, const std::wstring& delimiter) { return explode(s, delimiter.c_str()); }
	static std::vector<std::wstring> explodeMultiZero(const wchar_t *s);
	static std::vector<std::wstring> explodeQuoted(const wchar_t *s);
	static std::wstring              parseAscii(const BYTE *data, size_t length);
	static std::wstring              parseAscii(const std::vector<BYTE>& data) { return parseAscii(&data[0], data.size()); }
	static std::wstring              parseUtf8(const BYTE *data, size_t length);
	static std::wstring              parseUtf8(const std::vector<BYTE>& data)  { return parseUtf8(&data[0], data.size()); }
	static std::vector<BYTE>         serializeUtf8(const std::wstring& s);
};