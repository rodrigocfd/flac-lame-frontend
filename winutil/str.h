/**
 * Part of WinUtil - Windows Utilities
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winutil
 */

#pragma once
#include <string>
#include <vector>
#include <Windows.h>

namespace winutil {

struct str final {
	static std::wstring  format(const wchar_t* format, ...);
	static std::wstring& trim_nulls(std::wstring& s);
	static std::wstring& trim(std::wstring& s);
	static std::wstring  upper(const std::wstring& s);
	static std::wstring  lower(const std::wstring& s);
	static std::wstring& remove_diacritics(std::wstring& s);
	static bool          eqi(const std::wstring& s, const wchar_t* what)      { return !lstrcmpi(s.c_str(), what); }
	static bool          eqi(const std::wstring& s, const std::wstring& what) { return eqi(s.c_str(), what.c_str()); }
	static bool          ends_with(const std::wstring& s, const wchar_t* what);
	static bool          ends_with(const std::wstring& s, const std::wstring& what)    { return ends_with(s, what.c_str()); }
	static bool          ends_withi(const std::wstring& s, const wchar_t* what);
	static bool          ends_withi(const std::wstring& s, const std::wstring& what)   { return ends_withi(s, what.c_str()); }
	static bool          begins_with(const std::wstring& s, const wchar_t* what);
	static bool          begins_with(const std::wstring& s, const std::wstring& what)  { return begins_with(s, what.c_str()); }
	static bool          begins_withi(const std::wstring& s, const wchar_t* what);
	static bool          begins_withi(const std::wstring& s, const std::wstring& what) { return begins_withi(s, what.c_str()); }
	static size_t        findi(const std::wstring& s, const wchar_t* what, size_t offset = 0);
	static size_t        findi(const std::wstring& s, const std::wstring& what, size_t offset = 0)  { return findi(s, what.c_str(), offset); }
	static size_t        rfindi(const std::wstring& s, const wchar_t* what, size_t offset = 0);
	static size_t        rfindi(const std::wstring& s, const std::wstring& what, size_t offset = 0) { return rfindi(s, what.c_str(), offset); }
	static std::wstring& replace(std::wstring& haystack, const wchar_t* needle, const wchar_t* replacement);
	static std::wstring& replacei(std::wstring& haystack, const wchar_t* needle, const wchar_t* replacement);
	static bool          is_int(const std::wstring& s);
	static bool          is_uint(const std::wstring& s);
	static bool          is_hex(const std::wstring& s);
	static bool          is_float(const std::wstring& s);
	static std::vector<std::wstring> explode(const std::wstring& s, const wchar_t* delimiter);
	static std::vector<std::wstring> explode(const std::wstring& s, const std::wstring& delimiter) { return explode(s, delimiter.c_str()); }
	static std::vector<std::wstring> explode_multi_zero(const wchar_t* s);
	static std::vector<std::wstring> explode_quoted(const wchar_t* s);
	static std::wstring              parse_ascii(const BYTE* data, size_t length);
	static std::wstring              parse_ascii(const std::vector<BYTE>& data) { return parse_ascii(&data[0], data.size()); }
	static std::wstring              parse_utf8(const BYTE* data, size_t length);
	static std::wstring              parse_utf8(const std::vector<BYTE>& data)  { return parse_utf8(&data[0], data.size()); }
	static std::vector<BYTE>         serialize_utf8(const std::wstring& s);
};

}//namespace winutil