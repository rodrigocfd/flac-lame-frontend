
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

	std::copy(std::next(s.begin(), iFirst), // move the non-space chars back
		std::next(s.begin(), iLast + 1), s.begin());
	s.resize(iLast - iFirst + 1); // trim container size
	return s;
}

wstring& Str::upper(wstring& s)
{
	for (wchar_t& ch : s) {
		ch = towupper(ch);
	}
	return s;
}

wstring& Str::lower(wstring& s)
{
	for (wchar_t& ch : s) {
		ch = towlower(ch);
	}
	return s;
}

wstring& Str::removeDiacritics(wstring& s)
{
	const wchar_t *diacritics   = L"¡·¿‡√„¬‚ƒ‰…È»Ë ÍÀÎÕÌÃÏŒÓœÔ”Û“Ú’ı‘Ù÷ˆ⁄˙Ÿ˘€˚‹¸«Á≈Â–—Òÿ¯›˝";
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

wstring Str::folderFromPath(const wstring& path)
{
	wstring ret(path);
	ret.resize(ret.find_last_of(L'\\')); // also remove trailing backslash
	return ret;
}

wstring Str::fileFromPath(const wstring& path)
{
	wstring ret(path);
	ret.erase(0, ret.find_last_of(L'\\') + 1);
	return ret;
}

size_t Str::find(const wstring& s, const wchar_t *what, size_t offset)
{
	if (!s.size() || offset >= s.size()) {
		return wstring::npos; // same behavior of find_first_of
	}

	size_t whatLen = lstrlen(what);
	if (!whatLen || whatLen > s.size() - offset) {
		return wstring::npos;
	}

	size_t found = s.find_first_of(what[0], offset);
	while (found != wstring::npos && found + whatLen <= s.size()) {
		if (!memcmp(&s[found], what, whatLen * sizeof(wchar_t))) {
			return found;
		}
		found = s.find_first_of(what[0], found + 1);
	}

	return wstring::npos; // not found
}

size_t Str::findI(const wstring& s, const wchar_t *what, size_t offset)
{
	wstring haystack(s), needle(what);
	upper(haystack);
	upper(needle);
	return find(haystack, needle.c_str(), offset);
}

size_t Str::findLast(const wstring& s, const wchar_t *what, size_t offset)
{
	if (!s.size()) return wstring::npos;

	size_t whatLen = lstrlen(what);
	if (!whatLen || whatLen > s.size() - offset)
	if (!whatLen || whatLen - 1 > min(s.size() - 1, offset)) return wstring::npos; // same behavior of find_last_of

	size_t found = s.find_last_of(what[0], min(s.size() - whatLen, offset));
	while (found != wstring::npos) {
		if (!memcmp(&s[found], what, whatLen * sizeof(wchar_t))) {
			return found;
		}
		if (!found) break;
		found = s.find_last_of(what[0], found - 1);
	}

	return wstring::npos; // not found
}

size_t Str::findLastI(const wstring& s, const wchar_t *what, size_t offset)
{
	wstring haystack(s), needle(what);
	upper(haystack);
	upper(needle);
	return findLast(haystack, needle.c_str(), offset);
}

wstring& Str::replace(wstring& s, const wchar_t *what, const wchar_t *replacement)
{
	if (!s.size()) return s;

	size_t whatLen = lstrlen(what);
	if (!whatLen) return s;

	size_t replacementLen = lstrlen(replacement);
	wstring output;
	size_t base = 0;
	size_t found = 0;
	
	for (;;) {
		found = find(s, what, found);
		output.insert(output.size(), s, base, found - base);
		if (found != wstring::npos) {
			output.append(replacement);
			base = found = found + whatLen;
		} else {
			break;
		}
	}

	s.swap(output);
	return s;
}

wstring& Str::replaceI(wstring& s, const wchar_t *what, const wchar_t *replacement)
{
	if (!s.size()) return s;

	size_t whatLen = lstrlen(what);
	if (!whatLen) return s;

	wstring haystack(s), needle(what); // clone and uppercase
	upper(haystack);
	upper(needle);
	
	size_t replacementLen = lstrlen(replacement);
	wstring output;
	size_t base = 0;
	size_t found = 0;

	for (;;) {
		found = find(haystack, needle.c_str(), found);
		output.insert(output.size(), s, base, found - base);
		if (found != wstring::npos) {
			output.append(replacement);
			base = found = found + whatLen;
		} else {
			break;
		}
	}

	s.swap(output);
	return s;
}

bool Str::eq(const wstring& s, const wchar_t *what)
{
	return !lstrcmp(s.c_str(), what);
}

bool Str::eq(const wstring& s, const wstring& what)
{
	return eq(s, what.c_str());
}

bool Str::eqI(const wstring& s, const wchar_t *what)
{
	return !lstrcmpi(s.c_str(), what);
}

bool Str::eqI(const wstring& s, const wstring& what)
{
	return eqI(s, what.c_str());
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

	return !lstrcmp(&s[s.size() - whatLen], what);
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

	return !lstrcmpi(&s[s.size() - whatLen], what);
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

bool Str::isFloat(const wstring& s, float *pNum)
{
	if (s.empty()) return false;
	wchar_t *p = nullptr;
	float converted = wcstof(s.c_str(), &p);
	if (pNum && p != s.c_str()) *pNum = converted;
	return p != s.c_str();
}

bool Str::isFloatU(const wstring& s, float *pNum)
{
	if (s.empty() || s[0] == L'-') return false;
	wchar_t *p = nullptr;
	float converted = wcstof(s.c_str(), &p);
	if (pNum && p != s.c_str()) *pNum = converted;
	return p != s.c_str();
}

bool Str::isDouble(const wstring& s, double *pNum)
{
	if (s.empty()) return false;
	wchar_t *p = nullptr;
	double converted = wcstod(s.c_str(), &p);
	if (pNum && p != s.c_str()) *pNum = converted;
	return p != s.c_str();
}

bool Str::isDoubleU(const wstring& s, double *pNum)
{
	if (s.empty() || s[0] == L'-') return false;
	wchar_t *p = nullptr;
	double converted = wcstod(s.c_str(), &p);
	if (pNum && p != s.c_str()) *pNum = converted;
	return p != s.c_str();
}

bool Str::isInt(const wstring& s, int *pNum)
{
	if (s.empty()) return false;
	wchar_t *p = nullptr;
	int converted = wcstol(s.c_str(), &p, 10);
	if (pNum && p != s.c_str()) *pNum = converted;
	return p != s.c_str();
}

bool Str::isIntU(const wstring& s, unsigned int *pNum)
{
	if (s.empty() || s[0] == L'-') return false;
	wchar_t *p = nullptr;
	unsigned int converted = wcstoul(s.c_str(), &p, 10);
	if (pNum && p != s.c_str()) *pNum = converted;
	return p != s.c_str();
}

bool Str::isHex(const wstring& s, int *pNum)
{
	if (s.empty()) return false;
	wchar_t *p = nullptr;
	int converted = wcstol(s.c_str(), &p, 16);
	if (pNum && p != s.c_str()) *pNum = converted;
	return p != s.c_str();
}

bool Str::isHexU(const wstring& s, unsigned int *pNum)
{
	if (s.empty() || s[0] == L'-') return false;
	wchar_t *p = nullptr;
	unsigned int converted = wcstoul(s.c_str(), &p, 16);
	if (pNum && p != s.c_str()) *pNum = converted;
	return p != s.c_str();
}

vector<wstring> Str::explode(const wchar_t *s, const wchar_t *delimiters)
{
	// Count how many pieces we'll have after exploding.
	int num = 0;
	const wchar_t *pBase = s;
	for (;;) {
		size_t lenSub = wcscspn(pBase, delimiters);
		if (lenSub) ++num;
		if (pBase[lenSub] == L'\0') break;
		pBase += lenSub + 1;
	}

	vector<wstring> ret(num); // alloc return buffer

	// Grab each substring after explosion.
	num = 0;
	pBase = s;
	for (;;) {
		size_t lenSub = wcscspn(pBase, delimiters);
		if (lenSub) ret[num++].insert(0, pBase, lenSub);
		if (pBase[lenSub] == L'\0') break;
		pBase += lenSub + 1;
	}
	return ret;
}

vector<wstring> Str::explode(const wstring& s, const wchar_t *delimiters)
{
	return explode(s.c_str(), delimiters);
}

Str::Encoding Str::getEncoding(const BYTE *pData, size_t sz)
{
	auto match = [pData, sz](const BYTE *pBom, size_t szBom)->bool {
		return (sz >= szBom) &&
			!memcmp(pData, pBom, sizeof(BYTE) * szBom);
	};

	// https://en.wikipedia.org/wiki/Byte_order_mark

	BYTE utf8[] = { 0xEF, 0xBB, 0xBF };
	if (match(utf8, 3)) return Encoding::UTF8;

	BYTE utf16be[] = { 0xFE, 0xFF };
	if (match(utf16be, 2)) return Encoding::UTF16BE;

	BYTE utf16le[] = { 0xFF, 0xFE };
	if (match(utf16le, 2)) return Encoding::UTF16LE;
	
	BYTE utf32be[] = { 0x00, 0x00, 0xFE, 0xFF };
	if (match(utf32be, 4)) return Encoding::UTF32BE;

	BYTE utf32le[] = { 0xFF, 0xFE, 0x00, 0x00 };
	if (match(utf32le, 4)) return Encoding::UTF32LE;

	BYTE scsu[] = { 0x0E, 0xFE, 0xFF };
	if (match(scsu, 4)) return Encoding::SCSU;

	BYTE bocu1[] = { 0xFB, 0xEE, 0x28 };
	if (match(bocu1, 4)) return Encoding::BOCU1;

	return Encoding::ASCII;
}

Str::Encoding Str::getEncoding(const std::vector<BYTE>& data)
{
	return getEncoding(&data[0], data.size());
}