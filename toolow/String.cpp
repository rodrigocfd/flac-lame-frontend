
#pragma warning(disable:4996) // _vsnwprintf
#include <stdio.h>
#include "String.h"

#define WCHAR_ALLOC_STACK_OR_HEAP(nchars) \
	( ((nchars) * sizeof(wchar_t) > 1024 * 2) ? \
		(wchar_t*)::malloc(sizeof(wchar_t) * (nchars)) : \
		(wchar_t*)::_alloca(sizeof(wchar_t) * (nchars)) )

#define WCHAR_FREE_STACK_OR_HEAP(ptr, nchars) \
	if((nchars) * sizeof(wchar_t) > 1024 * 2) \
		::free(ptr)

#define WCHAR_CPY(dest, src, nchars) \
	::memcpy(dest, src, sizeof(wchar_t) * (nchars))

String& String::operator=(const wchar_t *s)
{
	int newLen = ::lstrlen(s);
	if(!_arr.size() && !newLen) return *this; // still unallocated and no new string: do nothing
	this->reserve(newLen); // also places terminating null
	WCHAR_CPY(&_arr[0], s, newLen);
	return *this;
}

String& String::reserve(int numCharsWithoutNull)
{
	if(numCharsWithoutNull > _arr.size() - 1) { // grow only
		bool firstAlloc = _arr.size() == 0;
		_arr.realloc(numCharsWithoutNull + 1); // also make room for terminating null
		if(firstAlloc)
			_arr[0] = L'\0';
	}
	_arr[numCharsWithoutNull] = L'\0'; // place terminating null, always
	return *this;
}

String& String::append(const wchar_t *s)
{
	if(!_arr.size()) return operator=(s);
	int ourLen = this->len();
	int theirLen = ::lstrlen(s);
	this->reserve(ourLen + theirLen); // also places terminating null
	WCHAR_CPY(&_arr[ourLen], s, theirLen);
	return *this;
}

String& String::append(std::initializer_list<const wchar_t*> arr)
{
	int *newLens = (int*)::_alloca(sizeof(int) * arr.size());
	int moreLen = 0;
	for(int i = 0, tot = (int)arr.size(); i < tot; ++i)
		moreLen += ( newLens[i] = ::lstrlen(*(arr.begin() + i)) );
	
	int ourLen = this->len();
	this->reserve(ourLen + moreLen);
	wchar_t *pRun = &_arr[ourLen];
	for(int i = 0, tot = (int)arr.size(); i < tot; ++i) {
		WCHAR_CPY(pRun, *(arr.begin() + i), newLens[i]);
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
	if(at < 0) at = 0;
	if(at >= this->len()) return this->append(s);
	_arr.insert(at, s, ::lstrlen(s));
	return *this;
}

String& String::formatv(const wchar_t *fmt, va_list args, int at)
{
	int newLen = ::_vscwprintf(fmt, args); // calculate length, without terminating null
	this->reserve(newLen + at);
	::_vsnwprintf(&_arr[at], newLen, fmt, args); // do the job
	return *this;
}

String& String::format(const wchar_t *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	this->formatv(fmt, args);
	va_end(args);
	return *this;
}

String& String::appendFormat(const wchar_t *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	this->formatv(fmt, args, this->len());
	va_end(args);
	return *this;
}

String String::substr(int start) const
{
	// Following the behaviour of http://www.php.net/substr .
	int ourLen = this->len();
	String ret;
	if(this->isEmpty()) {
		return ret;
	} else if(start >= 0) {
		if(start > ourLen - 1) return ret; // index out of bounds
		ret.copyFrom(&_arr[start], ourLen - start);
	} else {
		if(-start > ourLen) return ret; // index out of bounds
		ret.copyFrom(&_arr[ourLen + start], -start);
	}
	return ret;
}

String String::substr(int start, int length) const
{
	// Following the behaviour of http://www.php.net/substr .
	int ourLen = this->len();
	String ret;
	if(!length || this->isEmpty()) {
		return ret;
	} else if(start >= 0) { // positive start
		if(start > ourLen - 1) return ret; // index out of bounds
		if(length > 0) {
			if(start + length > ourLen) length = ourLen - start;
			ret.copyFrom(&_arr[start], length);
		} else {
			if(-length >= ourLen - start) return ret; // nothing to copy
			ret.copyFrom(&_arr[start], ourLen - start + length);
		}
	} else { // negative start
		if(-start > ourLen) return ret; // index out of bounds
		if(length > 0) {
			if(length > -start) length = -start;
			ret.copyFrom(&_arr[ourLen + start], length);
		} else {
			if(-length >= -start) return ret; // nothing to copy
			ret.copyFrom(&_arr[ourLen + start], (-start) + length);
		}
	}
	return ret;
}

String& String::copyFrom(const wchar_t *src, int numChars)
{
	this->reserve(numChars); // will also place terminating null
	WCHAR_CPY(&_arr[0], src, numChars); // won't check for buffer overruns
	return *this;
}

String& String::appendFrom(const wchar_t *src, int numChars)
{
	int ourLen = this->len();
	this->reserve(ourLen + numChars); // will also place terminating null
	WCHAR_CPY(&_arr[ourLen], src, numChars); // won't check for buffer overruns
	return *this;
}

String& String::trim()
{
	int len = this->len();
	int iFirst = 0, iLast = len - 1; // bounds
	bool onlySpaces = true;
	
	for(int i = 0; i < len; ++i) {
		if(!::iswspace(_arr[i])) {
			iFirst = i;
			onlySpaces = false;
			break;
		}
	}
	if(onlySpaces) {
		if(_arr.size()) _arr[0] = L'\0';
		return *this;
	}

	for(int i = len - 1; i >= 0; --i) {
		if(!::iswspace(_arr[i])) {
			iLast = i;
			break;
		}
	}
	::memmove(&_arr[0], &_arr[iFirst], sizeof(wchar_t) * (iLast - iFirst + 1));
	_arr[iLast - iFirst + 1] = L'\0';
	return *this;
}

bool String::beginsWith(const wchar_t *s, String::Case c, String::Diacritics d) const
{
	if(this->isEmpty()) return false;

	int theirLen = ::lstrlen(s);
	if(theirLen > this->len()) return false;

	wchar_t *ourBuf = WCHAR_ALLOC_STACK_OR_HEAP(theirLen + 1);
	WCHAR_CPY(ourBuf, &_arr[0], theirLen);
	ourBuf[theirLen] = L'\0';
	bool bRet = !this->LexicalCompare(ourBuf, s, c, d);
	WCHAR_FREE_STACK_OR_HEAP(ourBuf, theirLen + 1);
	
	return bRet;
}

bool String::endsWith(const wchar_t *s, Case c, Diacritics d) const
{
	if(this->isEmpty()) return false;

	int ourLen = this->len();
	int theirLen = ::lstrlen(s);
	if(theirLen > ourLen) return false;

	return !this->LexicalCompare(&_arr[ourLen - theirLen], s, c, d);
}

bool String::endsWith(wchar_t ch) const
{
	int ourLen = this->len();
	if(ourLen) return _arr[ourLen - 1] == ch;
	return false;
}

bool String::isInt() const
{
	int len = this->len();
	if(!len) return false;

	bool is = true;
	for(int i = 0; i < len; ++i) {
		if(!( _arr[i] >= L'0' && _arr[i] <= L'9' )) {
			is = false;
			break;
		}
	}
	return is;
}

bool String::isFloat() const
{
	int len = this->len();
	if(!len) return false;

	bool is = true, dotFound = false;
	for(int i = 0; i < len; ++i) {
		if(_arr[i] == L'.') {
			if(dotFound) {
				is = false;
				break;
			} else {
				dotFound = true; // allows only 1 dot character
			}
		} else if(!( _arr[i] >= L'0' && _arr[i] <= L'9' )) {
			is = false;
			break;
		}
	}
	return is;
}

int String::find(wchar_t ch) const
{
	const wchar_t *p = ::wcschr(this->str(), ch);
	if(!p) return -1; // not found
	return (int)(p - &_arr[0]); // return index of position
}

int String::find(const wchar_t *substring, String::Case c, String::Diacritics d) const
{
	wchar_t *ourBuf = NULL,       *theirBuf = NULL;
	int      ourLen = this->len(), theirLen = ::lstrlen(substring);

	if(!ourLen || theirLen > ourLen) return -1;

	if(c == Case::INSENS || d == Diacritics::REM) { // duplicate both strings to normalize
		DWORD flags = LCMAP_LINGUISTIC_CASING | LCMAP_LOWERCASE;
		
		ourBuf = WCHAR_ALLOC_STACK_OR_HEAP(ourLen + 1);
		if(c == Case::INSENS) ::LCMapString(LOCALE_SYSTEM_DEFAULT, flags, this->str(), ourLen + 1, ourBuf, ourLen + 1);
			else WCHAR_CPY(ourBuf, this->str(), ourLen + 1);
		if(d == Diacritics::REM) this->_RemDiacr(ourBuf);

		theirBuf = WCHAR_ALLOC_STACK_OR_HEAP(theirLen + 1);
		if(c == Case::INSENS) ::LCMapString(LOCALE_SYSTEM_DEFAULT, flags, substring, theirLen + 1, theirBuf, theirLen + 1);
			else WCHAR_CPY(theirBuf, substring, theirLen + 1);
		if(d == Diacritics::REM) this->_RemDiacr(theirBuf);
	} else {
		ourBuf = (wchar_t*)this->str();
		theirBuf = (wchar_t*)substring;
	}

	int iRet = -1;
	wchar_t *p = ::wcsstr(ourBuf, theirBuf);
	if(p) iRet = (int)(p - ourBuf); // index of position

	if(c == Case::INSENS || d == Diacritics::REM) {
		WCHAR_FREE_STACK_OR_HEAP(ourBuf, ourLen + 1);
		WCHAR_FREE_STACK_OR_HEAP(theirBuf, theirLen + 1);
	}

	return iRet;
}

int String::findr(wchar_t ch) const
{
	const wchar_t *p = ::wcsrchr(this->str(), ch);
	if(!p) return -1; // not found
	return (int)(p - &_arr[0]); // return index of position
}

String& String::replace(const wchar_t *target, const wchar_t *replacement, String::Case c, String::Diacritics d)
{
	wchar_t *ourBuf = NULL,       *targBuf = NULL;
	int      ourLen = this->len(), targLen = ::lstrlen(target);

	if(c == Case::INSENS || d == Diacritics::REM) { // duplicate str() and target to normalize
		DWORD flags = LCMAP_LINGUISTIC_CASING | LCMAP_LOWERCASE; // diacritics removal not supported on WinXP
		
		ourBuf = WCHAR_ALLOC_STACK_OR_HEAP(ourLen + 1);
		if(c == Case::INSENS) ::LCMapString(LOCALE_SYSTEM_DEFAULT, flags, this->str(), ourLen + 1, ourBuf, ourLen + 1);
			else WCHAR_CPY(ourBuf, this->str(), ourLen + 1);
		if(d == Diacritics::REM) this->_RemDiacr(ourBuf);

		targBuf = WCHAR_ALLOC_STACK_OR_HEAP(targLen + 1);
		if(c == Case::INSENS) ::LCMapString(LOCALE_SYSTEM_DEFAULT, flags, target, targLen + 1, targBuf, targLen + 1);
			else WCHAR_CPY(targBuf, target, targLen + 1);
		if(d == Diacritics::REM) this->_RemDiacr(targBuf);
	} else {
		ourBuf = (wchar_t*)this->str();
		targBuf = (wchar_t*)target;
	}

	int replacementLen = ::lstrlen(replacement);

	// Count occurrences of target.
	int occurrences = 0;
	const wchar_t *p = ourBuf;
	while(p = ::wcsstr(p, targBuf)) {
		++occurrences;
		p += targLen; // go beyond
	}

	// Alloc exact buffer to receive the replaced string.
	String finalBuf;
	finalBuf.reserve(ourLen + occurrences * (replacementLen - targLen));

	// Copy the string with the replacements made.
	wchar_t *pFinBuf = &finalBuf[0];
	const wchar_t *base = ourBuf, *orig = &_arr[0];
	p = ourBuf;
	while(p = ::wcsstr(p, targBuf)) {
		WCHAR_CPY(pFinBuf, orig, (int)(p - base)); // copy chars until before replacement
		pFinBuf += p - base;
		WCHAR_CPY(pFinBuf, replacement, replacementLen);
		pFinBuf += replacementLen;
		p += targLen;
		orig += p - base;
		base = p;
	}
	WCHAR_CPY(pFinBuf, orig, (int)(&_arr[0] + ourLen - orig)); // copy rest of original string, including terminating null
	
	_arr.swap(finalBuf._arr); // now the new buf is us, our old buffer will be released

	if(c == Case::INSENS || d == Diacritics::REM) {
		WCHAR_FREE_STACK_OR_HEAP(ourBuf, ourLen + 1);
		WCHAR_FREE_STACK_OR_HEAP(targBuf, targLen + 1);
	}
	return *this;
}

String& String::invert()
{
	if(_arr.size()) {
		for(int i = 0, j = this->len() - 1; i < j; ++i, --j) {
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
	for(;;) {
		int lenSub = (int)::wcscspn(pBase, delimiters);
		if(lenSub) ++num;
		if(pBase[lenSub] == L'\0') break;
		pBase += lenSub + 1;
	}
	
	Array<String> ret(num); // alloc return buffer

	// Grab each substring after explosion.
	num = 0;
	pBase = &_arr[0];
	for(;;) {
		int lenSub = (int)::wcscspn(pBase, delimiters);
		if(lenSub) ret[num++].copyFrom(pBase, lenSub);
		if(pBase[lenSub] == L'\0') break;
		pBase += lenSub + 1;
	}
	return ret;
}

String String::Format(const wchar_t *fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	String ret;
	ret.formatv(fmt, args);

	va_end(args);
	return ret;
}

String String::ParseUtf8(const BYTE *data, int length)
{
	String ret;
	int neededLen = ::MultiByteToWideChar(CP_UTF8, 0, (const char*)data, length, 0, 0);
	ret.reserve(neededLen);
	::MultiByteToWideChar(CP_UTF8, 0, (const char*)data, length, ret.ptrAt(0), neededLen);
	return ret;
}

Array<String> String::ExplodeQuoted(const wchar_t *quotedStr)
{
	// Example quotedStr:
	// "First one" NoQuoteSecond "Third one"

	// Count number of strings.
	int numStrings = 0;
	const wchar_t *pRun = quotedStr;
	while(*pRun) {
		if(*pRun == L'\"') { // begin of quoted string
			++pRun; // point to 1st char of string
			for(;;) {
				if(!*pRun) {
					break; // won't compute open-quoted
				} else if(*pRun == L'\"') {
					++pRun; // point to 1st char after closing quote
					++numStrings;
					break;
				}
				++pRun;
			}
		} else if(!::iswspace(*pRun)) { // 1st char of non-quoted string
			++pRun; // point to 2nd char of string
			while(*pRun && !::iswspace(*pRun) && *pRun != L'\"') ++pRun; // passed string
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
	while(*pRun) {
		if(*pRun == L'\"') { // begin of quoted string
			++pRun; // point to 1st char of string
			pBase = pRun;
			for(;;) {
				if(!*pRun) {
					break; // won't compute open-quoted
				} else if(*pRun == L'\"') {
					ret[i].copyFrom(pBase, (int)(pRun - pBase)); // copy to buffer
					++i; // next string

					++pRun; // point to 1st char after closing quote
					break;
				}
				++pRun;
			}
		} else if(!::iswspace(*pRun)) { // 1st char of non-quoted string
			pBase = pRun;
			++pRun; // point to 2nd char of string
			while(*pRun && !::iswspace(*pRun) && *pRun != L'\"') ++pRun; // passed string
			
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
	while(*pRun) {
		++numStrings;
		pRun += ::lstrlen(pRun) + 1;
	}

	// Alloc return array of strings.
	Array<String> ret(numStrings);

	// Copy each string.	
	pRun = multiStr;
	for(int i = 0; i < numStrings; ++i) {
		ret[i] = pRun;
		pRun += ::lstrlen(pRun) + 1;
	}

	return ret;
}

int String::LexicalCompare(const wchar_t *a, const wchar_t *b, Case c, Diacritics d)
{
	DWORD flags = 0;
	if(c == Case::INSENS) flags |= (NORM_IGNORECASE | NORM_IGNOREKANATYPE | NORM_LINGUISTIC_CASING);
	if(d == Diacritics::REM) flags |= NORM_IGNORENONSPACE;
	return ::CompareString(LOCALE_SYSTEM_DEFAULT, flags, a, -1, b, -1) - 2;
}

void String::_RemDiacr(wchar_t *txt)
{
	// The CompareString() function is the only one of its family
	// available on Windows XP.
	
	const wchar_t *diacritics   = L"ÁáÀàÃãÂâÄäÉéÈèÊêËëÍíÌìÎîÏïÓóÒòÕõÔôÖöÚúÙùÛûÜüÇçÅåÐðÑñØøÝý";
	const wchar_t *replacements = L"AaAaAaAaAaEeEeEeEeIiIiIiIiOoOoOoOoOoUuUuUuUuCcAaDdNnOoYy";

	wchar_t *pTxt = txt;
	while(*pTxt) {
		const wchar_t *pDiac = diacritics, *pRepl = replacements;
		while(*pDiac) {
			if(*pTxt == *pDiac) *pTxt = *pRepl; // in-place replacement
			++pDiac;
			++pRepl;
		}
		++pTxt;
	}
}

void dbg(const wchar_t *fmt, ...)
{
#ifdef _DEBUG
	va_list args;
	va_start(args, fmt);
	
	String buf;
	buf.formatv(fmt, args);
	
	va_end(args);
	::OutputDebugString(buf.str());
#endif
}