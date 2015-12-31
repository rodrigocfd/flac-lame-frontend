/**
 * Part of WOLF - WinAPI Object Lambda Framework
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/wolf
 */

#include "Str.h"
using namespace wolf;
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

wstring& Str::toUpper(wstring& s)
{
	for (wchar_t& ch : s) {
		ch = towupper(ch);
	}
	return s;
}

wstring& Str::toLower(wstring& s)
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
	if (!whatLen || whatLen > s.length()) {
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

	return !lstrcmp(&s[s.length() - whatLen], what);
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

	return !lstrcmpi(&s[s.length() - whatLen], what);
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