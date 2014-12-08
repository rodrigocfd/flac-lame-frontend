//
// Ordinary string implementation, ever growing, with buffer capabilities.
// Part of WOLF - Win32 Object Lambda Framework.
// @author Rodrigo Cesar de Freitas Dias
// @see https://github.com/rodrigocfd/wolf
//

#pragma warning(disable:4996) // _vsnwprintf
#include <stdio.h>
#include "String.h"

void dbg(const wchar_t *fmt, ...)
{
#ifdef _DEBUG
	va_list args;
	va_start(args, fmt);
	String buf = String::_Formatv(fmt, args);
	va_end(args);
	OutputDebugString(buf.str()); // no linebreak; if you want it, provide it on formatting string
#endif
}

void dbg(const String& s)
{
#ifdef _DEBUG
	OutputDebugString(s.str());
	OutputDebugString(L"\n");
#endif
}


String& String::operator=(const wchar_t *s)
{
	if (s) {
		int newLen = lstrlen(s);
		if (!_arr.size() && !newLen) return *this; // still unallocated and no new string: do nothing
		this->reserve(newLen); // also places terminating null
		memcpy(&_arr[0], s, sizeof(wchar_t) * newLen);
	}
	return *this;
}

String& String::reserve(int numCharsWithoutNull)
{
	if (numCharsWithoutNull > _arr.size() - 1) { // grow only
		bool firstAlloc = _arr.size() == 0;
		_arr.resize(numCharsWithoutNull + 1); // also make room for terminating null
		if (firstAlloc)
			_arr[0] = L'\0';
	}
	_arr[numCharsWithoutNull] = L'\0'; // place terminating null, always
	return *this;
}

String& String::append(const wchar_t *s)
{
	if (!_arr.size()) return operator=(s);
	int ourLen = this->len();
	int theirLen = lstrlen(s);
	this->reserve(ourLen + theirLen); // also places terminating null
	memcpy(&_arr[ourLen], s, sizeof(wchar_t) * theirLen);
	return *this;
}

String& String::append(initializer_list<const wchar_t*> arr)
{
	int *newLens = (int*)_alloca(sizeof(int) * arr.size()); // lenghts of each string to be appended
	int moreLen = 0; // sum of all lengthts
	for (int i = 0, tot = (int)arr.size(); i < tot; ++i)
		moreLen += ( newLens[i] = lstrlen(*(arr.begin() + i)) );
	
	int ourLen = this->len();
	this->reserve(ourLen + moreLen); // make room for all strings to be appended
	wchar_t *pRun = &_arr[ourLen];
	for (int i = 0, tot = (int)arr.size(); i < tot; ++i) {
		memcpy(pRun, *(arr.begin() + i), sizeof(wchar_t) * newLens[i]); // copy new string into place
		pRun += newLens[i];
	}

	return *this;
}

String& String::append(wchar_t ch)
{
	int newLen = this->len() + 1;
	this->reserve(newLen);
	_arr[newLen - 1] = ch;
	return *this;
}

String& String::insert(int at, const wchar_t *s)
{
	if (at < 0) at = 0;
	if (at >= this->len()) return this->append(s);
	_arr.insert(at, s, lstrlen(s));
	return *this;
}

String String::substr(int start) const
{
	// Following the behaviour of http://www.php.net/substr .
	int ourLen = this->len();
	String ret;
	if (this->isEmpty()) {
		return ret;
	} else if (start >= 0) {
		if (start > ourLen - 1) return ret; // index out of bounds
		ret.copyFrom(&_arr[start], ourLen - start);
	} else {
		if (-start > ourLen) return ret; // index out of bounds
		ret.copyFrom(&_arr[ourLen + start], -start);
	}
	return ret;
}

String String::substr(int start, int length) const
{
	// Following the behaviour of http://www.php.net/substr .
	int ourLen = this->len();
	String ret;
	if (!length || this->isEmpty()) {
		return ret;
	} else if (start >= 0) { // positive start
		if (start > ourLen - 1) return ret; // index out of bounds
		if (length > 0) {
			if (start + length > ourLen) length = ourLen - start;
			ret.copyFrom(&_arr[start], length);
		} else {
			if (-length >= ourLen - start) return ret; // nothing to copy
			ret.copyFrom(&_arr[start], ourLen - start + length);
		}
	} else { // negative start
		if (-start > ourLen) return ret; // index out of bounds
		if (length > 0) {
			if (length > -start) length = -start;
			ret.copyFrom(&_arr[ourLen + start], length);
		} else {
			if (-length >= -start) return ret; // nothing to copy
			ret.copyFrom(&_arr[ourLen + start], (-start) + length);
		}
	}
	return ret;
}

String& String::copyFrom(const wchar_t *src, int numChars)
{
	this->reserve(numChars); // will also place terminating null
	memcpy(&_arr[0], src, sizeof(wchar_t) * numChars); // won't check for buffer overruns
	return *this;
}

String& String::appendFrom(const wchar_t *src, int numChars)
{
	int ourLen = this->len();
	this->reserve(ourLen + numChars); // will also place terminating null
	memcpy(&_arr[ourLen], src, sizeof(wchar_t) * numChars); // won't check for buffer overruns
	return *this;
}

String& String::trim()
{
	int len = this->len();
	int iFirst = 0, iLast = len - 1; // bounds
	bool onlySpaces = true;
	
	for (int i = 0; i < len; ++i) {
		if (!iswspace(_arr[i])) {
			iFirst = i;
			onlySpaces = false;
			break;
		}
	}
	if (onlySpaces) {
		if (_arr.size()) _arr[0] = L'\0';
		return *this;
	}

	for (int i = len - 1; i >= 0; --i) {
		if (!iswspace(_arr[i])) {
			iLast = i;
			break;
		}
	}
	memmove(&_arr[0], &_arr[iFirst], sizeof(wchar_t) * (iLast - iFirst + 1));
	_arr[iLast - iFirst + 1] = L'\0';
	return *this;
}

String& String::removeDiacritics()
{
	if (!this->isEmpty()) {
		const wchar_t *diacritics   = L"ÁáÀàÃãÂâÄäÉéÈèÊêËëÍíÌìÎîÏïÓóÒòÕõÔôÖöÚúÙùÛûÜüÇçÅåÐðÑñØøÝýªº¹²³¢";
		const wchar_t *replacements = L"AaAaAaAaAaEeEeEeEeIiIiIiIiOoOoOoOoOoUuUuUuUuCcAaDdNnOoYyao123c";

		wchar_t *pTxt = &_arr[0];
		while (*pTxt) {
			const wchar_t *pDiac = diacritics, *pRepl = replacements;
			while (*pDiac) {
				if (*pTxt == *pDiac) *pTxt = *pRepl; // in-place replacement
				++pDiac; ++pRepl;
			}
			++pTxt;
		}
	}
	return *this;
}

bool String::endsWithCS(wchar_t ch) const
{
	int ourLen = this->len();
	if (ourLen) return _arr[ourLen - 1] == ch;
	return false;
}

bool String::isInt() const
{
	int len = this->len();
	if (!len) return false;

	bool is = true;
	for (int i = 0; i < len; ++i) {
		if (!( _arr[i] >= L'0' && _arr[i] <= L'9' )) {
			is = false;
			break;
		}
	}
	return is;
}

bool String::isFloat() const
{
	int len = this->len();
	if (!len) return false;

	bool is = true, dotFound = false;
	for (int i = 0; i < len; ++i) {
		if (_arr[i] == L'.') {
			if (dotFound) {
				is = false;
				break;
			} else {
				dotFound = true; // allows only 1 dot character
			}
		} else if (!( _arr[i] >= L'0' && _arr[i] <= L'9' )) {
			is = false;
			break;
		}
	}
	return is;
}

int String::findCS(wchar_t ch) const
{
	const wchar_t *p = wcschr(this->str(), ch);
	if (!p) return -1; // not found
	return (int)(p - &_arr[0]); // return index of position
}

int String::findrCS(wchar_t ch) const
{
	const wchar_t *p = wcsrchr(this->str(), ch);
	if (!p) return -1; // not found
	return (int)(p - &_arr[0]); // return index of position
}

String& String::invert()
{
	if (_arr.size()) {
		for (int i = 0, j = this->len() - 1; i < j; ++i, --j) {
			wchar_t ch = _arr[i];
			_arr[i] = _arr[j];
			_arr[j] = ch;
		}
	}
	return *this;
}

Array<String> String::explode(const wchar_t *delimiters) const
{
	// Count how many pieces we'll have after exploding.
	int num = 0;
	const wchar_t *pBase = &_arr[0];
	for (;;) {
		int lenSub = (int)wcscspn(pBase, delimiters);
		if (lenSub) ++num;
		if (pBase[lenSub] == L'\0') break;
		pBase += lenSub + 1;
	}
	
	Array<String> ret(num); // alloc return buffer

	// Grab each substring after explosion.
	num = 0;
	pBase = &_arr[0];
	for (;;) {
		int lenSub = (int)wcscspn(pBase, delimiters);
		if (lenSub) ret[num++].copyFrom(pBase, lenSub);
		if (pBase[lenSub] == L'\0') break;
		pBase += lenSub + 1;
	}
	return ret;
}

String String::Fmt(const wchar_t *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	String ret = _Formatv(fmt, args);
	va_end(args);
	return ret;
}

String String::ParseUtf8(const BYTE *data, int length)
{
	String ret;
	if (data && length) {
		int neededLen = MultiByteToWideChar(CP_UTF8, 0, (const char*)data, length, 0, 0);
		ret.reserve(neededLen);
		MultiByteToWideChar(CP_UTF8, 0, (const char*)data, length, ret.ptrAt(0), neededLen);
	}
	return ret;
}

Array<String> String::ExplodeQuoted(const wchar_t *quotedStr)
{
	// Example quotedStr:
	// "First one" NoQuoteSecond "Third one"

	// Count number of strings.
	int numStrings = 0;
	const wchar_t *pRun = quotedStr;
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
	Array<String> ret(numStrings);

	// Alloc and copy each string.
	pRun = quotedStr;
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
					ret[i].copyFrom(pBase, (int)(pRun - pBase)); // copy to buffer
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
			
			ret[i].copyFrom(pBase, (int)(pRun - pBase)); // copy to buffer
			++i; // next string
		} else {
			++pRun; // some white space
		}
	}

	return ret;
}

Array<String> String::ExplodeMulti(const wchar_t *multiStr)
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
	Array<String> ret(numStrings);

	// Copy each string.	
	pRun = multiStr;
	for (int i = 0; i < numStrings; ++i) {
		ret[i] = pRun;
		pRun += lstrlen(pRun) + 1;
	}

	return ret;
}

bool String::_beginsWith(const wchar_t *s, bool isCS) const
{
	if (this->isEmpty()) return false;

	int theirLen = lstrlen(s);
	if (theirLen > this->len()) return false;

	return _Compare(&_arr[0], s, isCS, theirLen) == 0;
}

bool String::_endsWith(const wchar_t *s, bool isCS) const
{
	if (this->isEmpty()) return false;

	int ourLen = this->len();
	int theirLen = lstrlen(s);
	if (theirLen > ourLen) return false;

	return _Compare(&_arr[ourLen - theirLen], s, isCS) == 0;
}

String& String::_replace(const wchar_t *target, const wchar_t *replacement, bool isCS)
{
	if (!target || !replacement || this->isEmpty()) return *this;

	int ourLen = this->len(),
		targLen = lstrlen(target),
		replLen = lstrlen(replacement);
	
	if (!targLen || targLen > ourLen) return *this;

	// Count occurrences of target.
	int occurrences = 0;
	const wchar_t *p = &_arr[0];
	while (p = _Find(p, target, isCS)) {
		++occurrences;
		p += targLen; // go beyond
	}

	// Alloc exact buffer to receive the replaced string.
	String finalBuf;
	finalBuf.reserve(ourLen + occurrences * (replLen - targLen));

	// Copy the string with the replacements made.
	wchar_t *pFinBuf = &finalBuf[0];
	const wchar_t *base = &_arr[0], *orig = &_arr[0];
	p = &_arr[0];
	while (p = _Find(p, target, isCS)) {
		memcpy(pFinBuf, orig, sizeof(wchar_t) * (p - base)); // copy chars until before replacement
		pFinBuf += p - base;
		memcpy(pFinBuf, replacement, sizeof(wchar_t) * replLen);
		pFinBuf += replLen;
		p += targLen;
		orig += p - base;
		base = p;
	}
	memcpy(pFinBuf, orig, sizeof(wchar_t) *
		(&_arr[0] + ourLen - orig)); // copy rest of original string, including terminating null
	
	_arr.swap(finalBuf._arr); // now the new buf is us, our old buffer will be released	
	return *this;
}

String String::_Formatv(const wchar_t *fmt, va_list args)
{
	int newLen = _vscwprintf(fmt, args); // calculate length, without terminating null
	String ret;
	ret.reserve(newLen);
	_vsnwprintf(ret.ptrAt(0), newLen, fmt, args); // do the job
	return ret;
}

wchar_t String::_ChangeCase(wchar_t ch, bool toUpper)
{
	if (toUpper && (
		(ch >= L'a' && ch <= L'z') ||
		(ch >= L'à' && ch <= L'ö') ||
		(ch >= L'ø' && ch <= L'þ') ))
	{
		return ch - 32;
	}
	else if (!toUpper && (
		(ch >= L'A' && ch <= L'Z') ||
		(ch >= L'À' && ch <= L'Ö') ||
		(ch >= L'Ø' && ch <= L'Þ') ))
	{
		return ch + 32;
	}
	return ch;
}

wchar_t* String::_ChangeCase(wchar_t *txt, bool toUpper)
{
	if (txt) {
		wchar_t *pTxt = txt;
		while (*pTxt) {
			*pTxt = _ChangeCase(*pTxt, toUpper);
			++pTxt;
		}
	}
	return txt;
}

int String::_Compare(const wchar_t *a, const wchar_t *b, bool isCS, int numCharsToSee)
{
	if (!a && !b) return 0;
	if (!a) return -1; else if (!b) return 1; // different strings

	int count = 0;
	for (;;) {
		if (!*a && !*b) return 0; // end of both strings reached

		if (isCS && *a != *b) {
			return (int)(*a - *b); // different strings
		} else if (!isCS) {
			wchar_t aa = _ChangeCase(*a, true), // cache uppercase
				bb = _ChangeCase(*b, true);
			if (aa != bb)
				return aa - bb; // different strings
		}

		++a; ++b; ++count;
		if (numCharsToSee && count == numCharsToSee) return 0;
	}
	return -42; // never happens
}

const wchar_t* String::_Find(const wchar_t *full, const wchar_t *what, bool isCS)
{
	int fullLen = lstrlen(full), whatLen = lstrlen(what);
	if (!fullLen || !whatLen || whatLen > fullLen) return nullptr;

	const wchar_t *ourBase = full;
	while (*ourBase) {
		const wchar_t *ours = ourBase, *theirs = what;
		for (;;) {			
			if (isCS && *ours != *theirs) break;
			else if (!isCS && _ChangeCase(*ours, true) != _ChangeCase(*theirs, true)) break;

			++ours; ++theirs;
			if (!*ours) return nullptr; // not found
			if (!*theirs) return ourBase;
		}
		++ourBase;
	}
	return nullptr; // not found
}