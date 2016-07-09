/**
 * Part of WinUtil - Windows Utilities
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winutil
 */

#include "str.h"
using namespace winutil;
using std::vector;
using std::wstring;

wstring str::format(const wchar_t* format, ...)
{
	va_list args;
	va_start(args, format);
	int newLen = _vscwprintf(format, args); // calculate length, without terminating null
	wstring ret(newLen, L'\0');
#pragma warning (disable: 4996)
	_vsnwprintf(&ret[0], newLen, format, args); // do the job
#pragma warning (default: 4996)
	va_end(args);
	return ret;
}

wstring& str::trim_nulls(wstring& s)
{
	// When a wstring is initialized with any length, possibly to be used as a buffer,
	// the string length may not match the size() method, after the operation.
	// This function fixes this.
	if (!s.empty()) {
		s.resize( lstrlen(s.c_str()) );
	}
	return s;
}

wstring& str::trim(wstring& s)
{
	if (s.empty()) {
		return s;
	}
	trim_nulls(s);

	size_t len = s.size();
	size_t iFirst = 0, iLast = len - 1; // bounds of trimmed string
	bool onlySpaces = true; // our string has only spaces?

	for (size_t i = 0; i < len; ++i) {
		if (!iswspace(s[i])) {
			iFirst = i;
			onlySpaces = false;
			break;
		}
	}
	if (onlySpaces) {
		s.clear();
		return s;
	}

	for (size_t i = len; i-- > 0; ) {
		if (!iswspace(s[i])) {
			iLast = i;
			break;
		}
	}

	std::copy(s.begin() + iFirst, // move the non-space chars back
		s.begin() + iLast + 1, s.begin());
	s.resize(iLast - iFirst + 1); // trim container size
	return s;
}

wstring str::upper(const wstring& s)
{
	wstring ret(s);
	CharUpperBuff(&ret[0], static_cast<DWORD>(ret.size()));
	return ret;
}

wstring str::lower(const wstring& s)
{
	wstring ret(s);
	CharLowerBuff(&ret[0], static_cast<DWORD>(ret.size()));
	return ret;
}

wstring& str::remove_diacritics(wstring& s)
{
	// Simple diacritics removal.

	const wchar_t* diacritics   = L"ÁáÀàÃãÂâÄäÉéÈèÊêËëÍíÌìÎîÏïÓóÒòÕõÔôÖöÚúÙùÛûÜüÇçÅåÐðÑñØøÝý";
	const wchar_t* replacements = L"AaAaAaAaAaEeEeEeEeIiIiIiIiOoOoOoOoOoUuUuUuUuCcAaDdNnOoYy";

	for (wchar_t& ch : s) {
		const wchar_t* pDiac = diacritics;
		const wchar_t* pRepl = replacements;
		while (*pDiac) {
			if (ch == *pDiac) ch = *pRepl; // in-place replacement
			++pDiac;
			++pRepl;
		}
	}
	return s;
}

static bool _firstEndsBeginsCheck(const wstring& s, const wchar_t* what, size_t& whatLen)
{
	if (s.empty()) {
		return false;
	}

	whatLen = lstrlen(what);
	if (!whatLen || whatLen > s.size()) {
		return false;
	}

	return true;
}

bool str::ends_with(const wstring& s, const wchar_t* what)
{
	size_t whatLen = 0;
	if (!_firstEndsBeginsCheck(s, what, whatLen)) {
		return false;
	}

	return !lstrcmp(s.c_str() + s.size() - whatLen, what);
}

bool str::ends_withi(const wstring& s, const wchar_t* what)
{
	size_t whatLen = 0;
	if (!_firstEndsBeginsCheck(s, what, whatLen)) {
		return false;
	}

	return !lstrcmpi(s.c_str() + s.size() - whatLen, what);
}

bool str::begins_with(const wstring& s, const wchar_t* what)
{
	size_t whatLen = 0;
	if (!_firstEndsBeginsCheck(s, what, whatLen)) {
		return false;
	}

	return !wcsncmp(s.c_str(), what, whatLen);
}

bool str::begins_withi(const wstring& s, const wchar_t* what)
{
	size_t whatLen = 0;
	if (!_firstEndsBeginsCheck(s, what, whatLen)) {
		return false;
	}

	return !_wcsnicmp(s.c_str(), what, whatLen);
}

size_t str::findi(const wstring& s, const wchar_t* what, size_t offset)
{
	wstring s2 = upper(s);
	wstring what2(what);
	CharUpperBuff(&what2[0], static_cast<DWORD>(what2.size()));
	return s2.find(what2, offset);
}

size_t str::rfindi(const wstring& s, const wchar_t* what, size_t offset)
{
	wstring s2 = upper(s);
	wstring what2(what);
	CharUpperBuff(&what2[0], static_cast<DWORD>(what2.size()));
	return s2.rfind(what2, offset);
}

wstring& str::replace(wstring& haystack, const wchar_t* needle, const wchar_t* replacement)
{
	if (haystack.empty()) return haystack;

	size_t needleLen = lstrlen(needle);
	if (!needleLen) return haystack;

	size_t replacementLen = lstrlen(replacement);
	wstring output;
	size_t base = 0;
	size_t found = 0;

	for (;;) {
		found = haystack.find(needle, found);
		//found = find(s, what, found);
		output.insert(output.size(), haystack, base, found - base);
		if (found != wstring::npos) {
			output.append(replacement);
			base = found = found + needleLen;
		} else {
			break;
		}
	}

	haystack.swap(output); // behaves like an in-place operation
	return haystack;
}

wstring& str::replacei(wstring& haystack, const wchar_t* needle, const wchar_t* replacement)
{
	if (!haystack.size()) return haystack;

	size_t needleLen = lstrlen(needle);
	if (!needleLen) return haystack;

	wstring haystackU = upper(haystack);
	wstring needleU = upper(needle);

	size_t replacementLen = lstrlen(replacement);
	wstring output;
	size_t base = 0;
	size_t found = 0;

	for (;;) {
		found = haystackU.find(needleU, found);
		output.insert(output.size(), haystack, base, found - base);
		if (found != wstring::npos) {
			output.append(replacement);
			base = found = found + needleLen;
		} else {
			break;
		}
	}

	haystack.swap(output); // behaves like an in-place operation
	return haystack;
}

bool str::is_int(const wstring& s)
{
	if (s.empty()) return false;
	if (s[0] != L'-' && !iswdigit(s[0]) && !iswblank(s[0])) return false;
	for (wchar_t ch : s) {
		if (!iswdigit(ch) && !iswblank(ch)) return false;
	}
	return true;
}

bool str::is_uint(const wstring& s)
{
	if (s.empty()) return false;
	for (wchar_t ch : s) {
		if (!iswdigit(ch) && !iswblank(ch)) return false;
	}
	return true;
}

bool str::is_hex(const wstring& s)
{
	if (s.empty()) return false;
	for (wchar_t ch : s) {
		if (!iswxdigit(ch) && !iswblank(ch)) return false;
	}
	return true;
}

bool str::is_float(const wstring& s)
{
	if (s.empty()) return false;
	if (s[0] != L'-' && s[0] != L'.' && !iswdigit(s[0]) && !iswblank(s[0])) return false;

	bool hasDot = false;
	for (wchar_t ch : s) {
		if (ch == L'.') {
			if (hasDot) {
				return false;
			} else {
				hasDot = true;
			}
		} else {
			if (!iswdigit(ch) && !iswblank(ch)) return false;
		}
	}
	return true;
}

wstring str::to_str_with_separator(int n, wchar_t separator)
{
	wstring ret;
	ret.reserve(16);

	int abso = abs(n);
	BYTE blocks = 0;
	while (abso >= 1000) {
		abso = (abso - (abso % 1000)) / 1000;
		++blocks;
	}

	abso = abs(n);
	bool firstPass = true;
	do {
		int num = abso % 1000;
		wchar_t buf[8] = { 0 };

		if (blocks) {
			if (num < 100) lstrcat(buf, L"0");
			if (num < 10) lstrcat(buf, L"0");
		}
		
		#pragma warning (disable: 4996)
		_itow(num, buf + lstrlen(buf), 10);
		#pragma warning (default: 4996)

		if (firstPass) {
			firstPass = false;
		} else {
			ret.insert(0, 1, separator);
		}

		ret.insert(0, buf);
		abso = (abso - (abso % 1000)) / 1000;
	} while (blocks--);

	if (n < 0) ret.insert(0, 1, L'-'); // prepend minus signal
	return ret;
}

vector<wstring> str::explode(const wstring& s, const wstring& delimiter)
{
	vector<wstring> ret;
	size_t base = 0, head = 0;

	for (;;) {
		head = s.find(delimiter, head);
		if (head == wstring::npos) break;
		ret.emplace_back();
		ret.back().insert(0, s, base, head - base);
		head += delimiter.size();
		base = head;
	}

	ret.emplace_back();
	ret.back().insert(0, s, base, s.size() - base);
	
	return ret;
}

vector<wstring> str::explode_multi_zero(const wchar_t* s)
{
	// Example multi-zero string:
	// L"first one\0second one\0third one\0"
	// Assumes a well-formed multiStr, which ends with two nulls.

	// Count number of null-delimited strings; string end with double null.
	size_t numStrings = 0;
	const wchar_t* pRun = s;
	while (*pRun) {
		++numStrings;
		pRun += lstrlen(pRun) + 1;
	}

	// Alloc return array of strings.
	vector<wstring> ret;
	ret.reserve(numStrings);

	// Copy each string.
	pRun = s;
	for (size_t i = 0; i < numStrings; ++i) {
		ret.emplace_back(pRun);
		pRun += lstrlen(pRun) + 1;
	}

	return ret;
}

vector<wstring> str::explode_quoted(const wchar_t* s)
{
	// Example quoted string:
	// "First one" NoQuoteSecond "Third one"

	// Count number of strings.
	size_t numStrings = 0;
	const wchar_t* pRun = s;
	while (*pRun) {
		if (*pRun == L'\"') { // begin of quoted string
			++pRun; // point to 1st char of string
			for (;;) {
				if (!*pRun) {
					break; // won't compute open-quoted
				} else if (*pRun == L'\"') {
					++pRun; // point to 1st char after closing quote
					++numStrings;
					break;
				}
				++pRun;
			}
		} else if (!iswspace(*pRun)) { // 1st char of non-quoted string
			++pRun; // point to 2nd char of string
			while (*pRun && !iswspace(*pRun) && *pRun != L'\"') ++pRun; // passed string
			++numStrings;
		} else {
			++pRun; // some white space
		}
	}

	// Alloc return array of strings.
	vector<wstring> ret;
	ret.reserve(numStrings);

	// Alloc and copy each string.
	pRun = s;
	const wchar_t* pBase;
	int i = 0;
	while (*pRun) {
		if (*pRun == L'\"') { // begin of quoted string
			++pRun; // point to 1st char of string
			pBase = pRun;
			for (;;) {
				if (!*pRun) {
					break; // won't compute open-quoted
				} else if (*pRun == L'\"') {
					ret.emplace_back();
					ret.back().insert(0, pBase, pRun - pBase); // copy to buffer
					++i; // next string

					++pRun; // point to 1st char after closing quote
					break;
				}
				++pRun;
			}
		} else if (!iswspace(*pRun)) { // 1st char of non-quoted string
			pBase = pRun;
			++pRun; // point to 2nd char of string
			while (*pRun && !iswspace(*pRun) && *pRun != L'\"') ++pRun; // passed string

			ret.emplace_back();
			ret.back().insert(0, pBase, pRun - pBase); // copy to buffer
			++i; // next string
		} else {
			++pRun; // some white space
		}
	}

	return ret;
}

wstring str::parse_ascii(const BYTE* data, size_t length)
{
	wstring ret;
	if (data && length) {
		ret.resize(length);
		for (size_t i = 0; i < length; ++i) {
			ret[i] = static_cast<wchar_t>(data[i]); // raw conversion
		}
	}
	return ret;
}

static wstring _parseEncoded(const BYTE* data, size_t length, UINT codePage)
{
	wstring ret;
	if (data && length) {
		int neededLen = MultiByteToWideChar(codePage, 0, reinterpret_cast<const char*>(data),
			static_cast<int>(length), nullptr, 0);
		ret.resize(neededLen);
		MultiByteToWideChar(codePage, 0, reinterpret_cast<const char*>(data),
			static_cast<int>(length), &ret[0], neededLen);
		str::trim_nulls(ret);
	}
	return ret;
}

wstring str::parse_win1252(const BYTE* data, size_t length)
{
	return _parseEncoded(data, length, 1252);
}

wstring str::parse_utf8(const BYTE* data, size_t length)
{
	return _parseEncoded(data, length, CP_UTF8);
}

vector<BYTE> str::serialize_utf8(const wstring& s)
{
	vector<BYTE> ret;
	if (!s.empty()) {
		int neededLen = WideCharToMultiByte(CP_UTF8, 0,
			s.c_str(), static_cast<int>(s.size()),
			nullptr, 0, nullptr, 0);
		ret.resize(neededLen);
		WideCharToMultiByte(CP_UTF8, 0,
			s.c_str(), static_cast<int>(s.size()), 
			reinterpret_cast<char*>(&ret[0]),
			neededLen, nullptr, nullptr);
	}
	return ret; // no BOM here
}