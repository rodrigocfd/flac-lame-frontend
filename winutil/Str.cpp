
#include "Str.h"
using std::vector;
using std::wstring;

wstring Str::format(const wchar_t *format, ...)
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

wstring& Str::trimNulls(wstring& s)
{
	// When a wstring is initialized with any length, possibly to be used as a buffer,
	// the string length may not match the size() method, after the operation.
	// This function fixes this.
	if (!s.empty()) {
		s.resize( lstrlen(s.c_str()) );
	}
	return s;
}

wstring& Str::trim(wstring& s)
{
	if (s.empty()) {
		return s;
	}
	trimNulls(s);

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

wstring Str::upper(const wstring& s)
{
	wstring ret(s);
	CharUpperBuff(&ret[0], static_cast<DWORD>(ret.size()));
	return ret;
}

wstring Str::lower(const wstring& s)
{
	wstring ret(s);
	CharLowerBuff(&ret[0], static_cast<DWORD>(ret.size()));
	return ret;
}

wstring& Str::removeDiacritics(wstring& s)
{
	const wchar_t *diacritics   = L"ÁáÀàÃãÂâÄäÉéÈèÊêËëÍíÌìÎîÏïÓóÒòÕõÔôÖöÚúÙùÛûÜüÇçÅåÐðÑñØøÝý";
	const wchar_t *replacements = L"AaAaAaAaAaEeEeEeEeIiIiIiIiOoOoOoOoOoUuUuUuUuCcAaDdNnOoYy";

	for (wchar_t& ch : s) {
		const wchar_t *pDiac = diacritics, *pRepl = replacements;
		while (*pDiac) {
			if (ch == *pDiac) ch = *pRepl; // in-place replacement
			++pDiac;
			++pRepl;
		}
	}
	return s;
}

bool Str::eqI(const wstring& s, const wchar_t *what)
{
	return !lstrcmpi(s.c_str(), what);
}

bool Str::eqI(const wstring& s, const wstring& what)
{
	return eqI(s.c_str(), what.c_str());
}

static bool _firstEndsBeginsCheck(const wstring& s, const wchar_t *what, size_t& whatLen)
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

bool Str::endsWith(const wstring& s, const wchar_t *what)
{
	size_t whatLen = 0;
	if (!_firstEndsBeginsCheck(s, what, whatLen)) {
		return false;
	}

	return !lstrcmp(s.c_str() + s.size() - whatLen, what);
}

bool Str::endsWith(const wstring& s, const wstring& what)
{
	return endsWith(s, what.c_str());
}

bool Str::endsWithI(const wstring& s, const wchar_t *what)
{
	size_t whatLen = 0;
	if (!_firstEndsBeginsCheck(s, what, whatLen)) {
		return false;
	}

	return !lstrcmpi(s.c_str() + s.size() - whatLen, what);
}

bool Str::endsWithI(const wstring& s, const wstring& what)
{
	return endsWithI(s, what.c_str());
}

bool Str::beginsWith(const wstring& s, const wchar_t *what)
{
	size_t whatLen = 0;
	if (!_firstEndsBeginsCheck(s, what, whatLen)) {
		return false;
	}

	return !wcsncmp(s.c_str(), what, whatLen);
}

bool Str::beginsWith(const wstring& s, const wstring& what)
{
	return beginsWith(s, what.c_str());
}

bool Str::beginsWithI(const wstring& s, const wchar_t *what)
{
	size_t whatLen = 0;
	if (!_firstEndsBeginsCheck(s, what, whatLen)) {
		return false;
	}

	return !_wcsnicmp(s.c_str(), what, whatLen);
}

bool Str::beginsWithI(const wstring& s, const wstring& what)
{
	return beginsWithI(s, what.c_str());
}

size_t Str::findI(const wstring& s, const wchar_t *what, size_t offset)
{
	wstring s2 = upper(s);
	wstring what2(what);
	CharUpperBuff(&what2[0], static_cast<DWORD>(what2.size()));
	return s2.find(what2, offset);
}

size_t Str::findI(const wstring& s, const wstring& what, size_t offset)
{
	return findI(s, what.c_str(), offset);
}

size_t Str::rfindI(const wstring& s, const wchar_t *what, size_t offset)
{
	wstring s2 = upper(s);
	wstring what2(what);
	CharUpperBuff(&what2[0], static_cast<DWORD>(what2.size()));
	return s2.rfind(what2, offset);
}

size_t Str::rfindI(const wstring& s, const wstring& what, size_t offset)
{
	return rfindI(s, what.c_str(), offset);
}

wstring& Str::replace(wstring& haystack, const wchar_t *needle, const wchar_t *replacement)
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

wstring& Str::replaceI(wstring& haystack, const wchar_t *needle, const wchar_t *replacement)
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

bool Str::isInt(const wstring& s)
{
	if (s.empty()) return false;
	if (s[0] != L'-' && !iswdigit(s[0]) && !iswblank(s[0])) return false;
	for (wchar_t ch : s) {
		if (!iswdigit(ch) && !iswblank(ch)) return false;
	}
	return true;
}

bool Str::isUint(const wstring& s)
{
	if (s.empty()) return false;
	for (wchar_t ch : s) {
		if (!iswdigit(ch) && !iswblank(ch)) return false;
	}
	return true;
}

bool Str::isHex(const wstring& s)
{
	if (s.empty()) return false;
	for (wchar_t ch : s) {
		if (!iswxdigit(ch) && !iswblank(ch)) return false;
	}
	return true;
}

bool Str::isFloat(const wstring& s)
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

vector<wstring> Str::explode(const wstring& s, const wchar_t *delimiter)
{
	vector<wstring> ret;
	size_t delimiterLen = lstrlen(delimiter);
	size_t base = 0, head = 0;

	for (;;) {
		head = s.find(delimiter, head);
		if (head == wstring::npos) break;
		ret.emplace_back();
		ret.back().insert(0, s, base, head - base);
		head += delimiterLen;
		base = head;
	}

	ret.emplace_back();
	ret.back().insert(0, s, base, s.size() - base);
	
	return ret;
}

vector<wstring> Str::explode(const wstring& s, const wstring& delimiter)
{
	return explode(s, delimiter.c_str());
}

vector<wstring> Str::explodeMultiZero(const wchar_t *s)
{
	// Example multiStr:
	// L"first one\0second one\0third one\0"
	// Assumes a well-formed multiStr, which ends with two nulls.

	// Count number of null-delimited strings; string end with double null.
	size_t numStrings = 0;
	const wchar_t *pRun = s;
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

vector<wstring> Str::explodeQuoted(const wchar_t *s)
{
	// Example quotedStr:
	// "First one" NoQuoteSecond "Third one"

	// Count number of strings.
	size_t numStrings = 0;
	const wchar_t *pRun = s;
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
	const wchar_t *pBase;
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

wstring Str::parseAscii(const BYTE *data, size_t length)
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

wstring Str::parseAscii(const std::vector<BYTE>& data)
{
	return parseAscii(&data[0], data.size());
}

wstring Str::parseUtf8(const BYTE *data, size_t length)
{
	wstring ret;
	if (data && length) {
		int neededLen = MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<const char*>(data),
			static_cast<int>(length), nullptr, 0);
		ret.resize(neededLen);
		MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<const char*>(data),
			static_cast<int>(length), &ret[0], neededLen);
		trimNulls(ret);
	}
	return ret;
}

wstring Str::parseUtf8(const vector<BYTE>& data)
{
	return parseUtf8(&data[0], data.size());
}

vector<BYTE> Str::serializeUtf8(const wstring& s)
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
	return ret;
}