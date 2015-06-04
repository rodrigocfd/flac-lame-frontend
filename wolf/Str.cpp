/*!
 * @file
 * @brief String and vector utilities.
 * @details Part of WOLF - Win32 Object Lambda Framework.
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/wolf
 */

#include <cwctype> // iswpace
#include "Str.h"
#pragma warning(disable:4996) // _vsnwprintf
using namespace wolf;
using std::wstring;
using std::vector;

void str::Dbg(const wchar_t *fmt, ...)
{
#ifdef _DEBUG
	va_list args;
	va_start(args, fmt);
	int newLen = _vscwprintf(fmt, args); // calculate length, without terminating null
	wstring buf(newLen, L'\0');
	_vsnwprintf(&buf[0], newLen, fmt, args); // do the job
	va_end(args);
	OutputDebugString(buf.c_str());
#endif
}

wstring str::Sprintf(const wchar_t *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	int newLen = _vscwprintf(fmt, args); // calculate length, without terminating null
	wstring ret(newLen, L'\0');
	_vsnwprintf(&ret[0], newLen, fmt, args); // do the job
	va_end(args);
	return ret;
}

wstring& str::Trim(wstring& s)
{
	if (s.empty()) return s;
	TrimNulls(s);

	size_t len = s.size();
	size_t iFirst = 0, iLast = len - 1; // bounds of trimmed string
	bool onlySpaces = true; // our string has only spaces?

	for (size_t i = 0; i < len; ++i) {
		if (!std::iswspace(s[i])) {
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
		if (!std::iswspace(s[i])) {
			iLast = i;
			break;
		}
	}

	std::copy(std::next(s.begin(), iFirst), // move the non-space chars back
		std::next(s.begin(), iLast + 1), s.begin());
	s.resize(iLast - iFirst + 1); // trim container size
	return s;
}

static wchar_t _ChangeCase(wchar_t ch, bool toUpper)
{
	if (toUpper && (
		(ch >= L'a' && ch <= L'z') ||
		(ch >= L'‡' && ch <= L'ˆ') ||
		(ch >= L'¯' && ch <= L'˛') ))
	{
		return ch - 32;
	}
	else if (!toUpper && (
		(ch >= L'A' && ch <= L'Z') ||
		(ch >= L'¿' && ch <= L'÷') ||
		(ch >= L'ÿ' && ch <= L'ﬁ') ))
	{
		return ch + 32;
	}
	return ch;
}

wstring& str::ToUpper(wstring& s)
{
	for (wchar_t& ch : s) {
		ch = _ChangeCase(ch, true);
	}
	return s;
}

wstring& str::ToLower(wstring& s)
{
	for (wchar_t& ch : s) {
		ch = _ChangeCase(ch, false);
	}
	return s;
}

bool str::IsInt(const wstring& s)
{
	if (s.empty()) return false;

	for (const wchar_t& ch : s) {
		if (ch < L'0' || ch > L'9') {
			return false;
		}
	}
	return true;
}

bool str::IsFloat(const wstring& s)
{
	if (s.empty()) return false;

	bool dotFound = false;
	for (const wchar_t& ch : s) {
		if (ch == L'.') {
			if (dotFound) {
				return false;
			} else {
				dotFound = true; // allows only 1 dot character
			}
		} else if (ch < L'0' || ch > L'9') {
			return false;
		}
	}
	return true;
}

int str::LexCmp(Sens se, const wchar_t *a, const wchar_t *b, size_t nChars)
{
	if (!a && !b) return 0;
	if (!a) return -1; else if (!b) return 1; // different strings

	int count = 0;
	for (;;) {
		if (!*a && !*b) return 0; // end of both strings reached

		if (se == Sens::YES && *a != *b) {
			return static_cast<int>(*a - *b); // different strings
		} else if (se == Sens::NO) {
			wchar_t aa = _ChangeCase(*a, true), // cache uppercase
				bb = _ChangeCase(*b, true);
			if (aa != bb) return aa - bb; // different strings
		}

		++a; ++b; ++count;
		if (nChars && count == nChars) return 0;
	}
	return -42; // never happens
}

bool str::BeginsWith(Sens se, const wstring& s, const wchar_t *what)
{
	if (s.empty()) return false;

	size_t whatLen = lstrlen(what);
	if (!whatLen || whatLen > s.length()) return false;

	return !LexCmp(se, &s[0], what, whatLen);
}

bool str::EndsWith(Sens se, const wstring& s, const wchar_t *what)
{
	if (s.empty()) return false;

	size_t whatLen = lstrlen(what);
	if (!whatLen || whatLen > s.length()) return false;

	return !LexCmp(se, &s[s.length() - whatLen], what, whatLen);
}

static const wchar_t* _FindCh(str::Sens se, const wchar_t *s, size_t slen, wchar_t whatCh, bool isReverse)
{
	if (!slen) return nullptr;

	wchar_t bb = (se == str::Sens::YES) ? whatCh : _ChangeCase(whatCh, true);
	if (isReverse) {
		for (size_t i = slen; i-- > 0; ) {
			wchar_t aa = (se == str::Sens::YES) ? s[i] : _ChangeCase(s[i], true);
			if (aa == bb) {
				return s + i;
			}
		}
	} else {
		for (size_t i = 0; i < slen; ++i) {
			wchar_t aa = (se == str::Sens::YES) ? s[i] : _ChangeCase(s[i], true);
			if (aa == bb) {
				return s + i;
			}
		}
	}
	return nullptr; // not found
}

static const wchar_t* _FindStr(str::Sens se, const wchar_t *s, size_t slen, const wchar_t *whatStr, bool isReverse)
{
	if (!slen) return nullptr;

	int lenWhat = lstrlen(whatStr);
	if (lenWhat == 1) return _FindCh(se, s, slen, *whatStr, isReverse); // ordinary char search

	wchar_t *tmpWhat = nullptr;
	if (se == str::Sens::NO) { // create temp with uppercase version of "what"
		tmpWhat = static_cast<wchar_t*>(_alloca((lenWhat + 1) * sizeof(wchar_t)));
		for (int i = 0; i < lenWhat; ++i)
			tmpWhat[i] = _ChangeCase(whatStr[i], true);
		tmpWhat[lenWhat] = L'\0'; // trailing null
	} else {
		tmpWhat = const_cast<wchar_t*>(whatStr);
	}

	if (isReverse) {
		for (size_t lenS = slen; ; ) {
			const wchar_t *found = _FindCh(se, s, lenS, *tmpWhat, true);
			if (!found) break;
			const wchar_t *pS = found + 1,
				*pWhat = tmpWhat + 1;
			while (*pS && ((se == str::Sens::YES) ? *pS : _ChangeCase(*pS, true)) == *pWhat) {
				if (!*++pWhat) return found;
				++pS;
			}
			lenS = found - s;
		}
	} else {
		for (int i = 0; ; ) {
			const wchar_t *found = _FindCh(se, s + i, slen - i, *tmpWhat, false);
			if (!found) break;
			const wchar_t *pS = found + 1,
				*pWhat = tmpWhat + 1;
			while (*pS && ((se == str::Sens::YES) ? *pS : _ChangeCase(*pS, true)) == *pWhat) {
				if (!*++pWhat) return found;
				++pS;
			}
			++i;
		}
	}
	return nullptr; // not found
}

static int    _FindRetIdx(const wstring& s, const wchar_t *pFound) { return pFound ? static_cast<int>(pFound - s.c_str()) : -1; }
int            str::Find(Sens se, const wstring& s, wchar_t what)        { return _FindRetIdx(s, _FindCh(se, s.c_str(), s.length(), what, false)); }
const wchar_t* str::Find(Sens se, const wchar_t *s, const wchar_t *what) { return _FindStr(se, s, lstrlen(s), what, false); }
int            str::Find(Sens se, const wstring& s, const wchar_t *what) { return _FindRetIdx(s, _FindStr(se, s.c_str(), s.length(), what, false)); }
int            str::FindRev(Sens se, const wstring& s, wchar_t what)        { return _FindRetIdx(s, _FindCh(se, s.c_str(), s.length(), what, true)); }
const wchar_t* str::FindRev(Sens se, const wchar_t *s, const wchar_t *what) { return _FindStr(se, s, lstrlen(s), what, true); }
int            str::FindRev(Sens se, const wstring& s, const wchar_t *what) { return _FindRetIdx(s, _FindStr(se, s.c_str(), s.length(), what, true)); }

wstring& str::Replace(Sens se, wstring& s, const wchar_t *target, const wchar_t *replacement)
{
	if (!target || !replacement || s.empty()) return s;

	int ourLen = static_cast<int>(s.length()),
		targLen = lstrlen(target),
		replLen = lstrlen(replacement);
	
	if (!targLen || targLen > ourLen) return s;

	// Count occurrences of target.
	int occurrences = 0;
	const wchar_t *p = s.c_str();
	while (p = _FindStr(se, p, ourLen - (p - s.c_str()), target, false)) {
		++occurrences;
		p += targLen; // go beyond
	}
	if (!occurrences) return s;

	// Alloc exact buffer to receive the replaced string.
	wstring finalBuf(ourLen + occurrences * (replLen - targLen), L'\0');

	// Copy the string with the replacements made.
	wchar_t *pFinBuf = &finalBuf[0];
	const wchar_t *base = s.c_str(), *orig = s.c_str();
	p = s.c_str();
	while (p = _FindStr(se, p, ourLen - (p - s.c_str()), target, false)) {
		memcpy(pFinBuf, orig, sizeof(wchar_t) * (p - base)); // copy chars until before replacement
		pFinBuf += p - base;
		memcpy(pFinBuf, replacement, sizeof(wchar_t) * replLen);
		pFinBuf += replLen;
		p += targLen;
		orig += p - base;
		base = p;
	}
	memcpy(pFinBuf, orig, sizeof(wchar_t) *
		(s.c_str() + ourLen - orig)); // copy rest of original string, including terminating null
	
	s.swap(finalBuf); // now the new buf is us, our old buffer will be released
	return s;
}

wstring& str::RemoveDiacritics(wstring& s)
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

wstring str::ParseUtf8(const BYTE *data, size_t length)
{
	wstring ret;
	if (data && length) {
		int neededLen = MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<const char*>(data),
			static_cast<int>(length), nullptr, 0);
		ret.resize(neededLen);
		MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<const char*>(data),
			static_cast<int>(length), &ret[0], neededLen);
		TrimNulls(ret);
	}
	return ret;
}

vector<wstring> str::Explode(const wstring& s, const wchar_t *delimiters)
{
	// Count how many pieces we'll have after exploding.
	int num = 0;
	const wchar_t *pBase = s.c_str();
	for (;;) {
		size_t lenSub = wcscspn(pBase, delimiters);
		if (lenSub) ++num;
		if (pBase[lenSub] == L'\0') break;
		pBase += lenSub + 1;
	}
	
	vector<wstring> ret(num); // alloc return buffer

	// Grab each substring after explosion.
	num = 0;
	pBase = s.c_str();
	for (;;) {
		size_t lenSub = wcscspn(pBase, delimiters);
		if (lenSub) ret[num++].insert(0, pBase, lenSub);
		if (pBase[lenSub] == L'\0') break;
		pBase += lenSub + 1;
	}
	return ret;
}

vector<wstring> str::ExplodeMultiStr(const wchar_t *multiStr)
{
	// Example multiStr:
	// L"first one\0second one\0third one\0"
	// Assumes a well-formed multiStr, which ends with two nulls.

	// Count number of null-delimited strings; string end with double null.
	int numStrings = 0;
	const wchar_t *pRun = multiStr;
	while (*pRun) {
		++numStrings;
		pRun += lstrlen(pRun) + 1;
	}

	// Alloc return array of strings.
	vector<wstring> ret;
	ret.reserve(numStrings);

	// Copy each string.
	pRun = multiStr;
	for (int i = 0; i < numStrings; ++i) {
		ret.emplace_back(pRun);
		pRun += lstrlen(pRun) + 1;
	}

	return ret;
}