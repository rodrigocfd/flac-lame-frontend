
#pragma warning(disable:4996)
#include <stdio.h>
#include "String.h"

String& String::operator=(const wchar_t *s)
{
	int newLen = ::lstrlen(s);
	if(!_arr.size() && !newLen) return *this; // still unallocated and no new string: do nothing
	this->reserve(newLen);
	::lstrcpy(&_arr[0], s);
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

	_arr[numCharsWithoutNull] = L'\0'; // place terminating null
	return *this;
}

String& String::append(const wchar_t *s)
{
	if(!_arr.size()) return operator=(s);
	int ourLen = this->len();
	int newLen = ::lstrlen(s) + ourLen;
	this->reserve(newLen);
	::lstrcpy(&_arr[ourLen], s);
	return *this;
}

String& String::append(wchar_t ch)
{
	int newLen = this->len() + 1;
	this->reserve(newLen);
	_arr[newLen - 1] = ch;
	return *this;
}

String& String::fmtv(const wchar_t *fmt, va_list args, int at)
{
	int newLen = ::_vscwprintf(fmt, args); // calculate length, without terminating null
	this->reserve(newLen + at);
	::_vsnwprintf(&_arr[at], newLen, fmt, args); // do the job
	return *this;
}

String& String::fmt(const wchar_t *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	this->fmtv(fmt, args);
	va_end(args);
	return *this;
}

String& String::appendfmt(const wchar_t *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	this->fmtv(fmt, args, this->len());
	va_end(args);
	return *this;
}

String& String::copyFrom(const wchar_t *src, int numChars)
{
	this->reserve(numChars);
	::wcsncpy(&_arr[0], src, numChars);
	return *this;
}

String& String::appendFrom(const wchar_t *src, int numChars)
{
	int ourLen = this->len();
	this->reserve(ourLen + numChars);
	::wcsncpy(&_arr[ourLen], src, numChars);
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

bool String::endsWith(const wchar_t *s) const
{
	int ourLen = this->len(), hisLen = ::lstrlen(s);
	if(ourLen >= hisLen)
		return !::lstrcmpi(&_arr[ourLen - hisLen], s); // case insensitive
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

int String::findr(wchar_t ch) const
{
	const wchar_t *p = ::wcsrchr(this->str(), ch);
	if(!p) return -1; // not found
	return (int)(p - &_arr[0]); // return index of position
}

String& String::replace(const wchar_t *target, const wchar_t *replacement)
{
	int targetLen = ::lstrlen(target);
	int replacementLen = ::lstrlen(replacement);

	// Count occurrences of target.
	int occurrences = 0;
	const wchar_t *p = &_arr[0];
	while(p = ::wcsstr(p, target)) {
		++occurrences;
		p += targetLen; // go beyond
	}

	// Alloc exact buffer to receive the replaced string.
	String buf;
	buf.reserve((replacementLen - targetLen) * occurrences + this->len());

	// Copy the string with the replacements made.
	wchar_t *pBuf = &buf[0];
	const wchar_t *base = &_arr[0];
	p = &_arr[0];
	while(p = ::wcsstr(p, target)) {
		::wcsncpy(pBuf, base, (int)(p - base) + 1); // number of chars includes the terminating null
		pBuf += p - base; // go beyond
		::wcsncpy(pBuf, replacement, replacementLen + 1); // number of chars includes the terminating null
		pBuf += replacementLen; // go beyond
		p += targetLen; // go beyond
		base = p; // go beyond
	}
	::wcsncpy(pBuf, base, ::lstrlen(base) + 1); // copy rest of original string, including terminating null
	
	_arr.swap(&buf._arr); // now the new buf is us, our old buffer will be released
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

void String::explode(const wchar_t *delimiters, Array<String> *pBuf)
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
	pBuf->realloc(num); // realloc return buffer

	// Grab each substring after explosion.
	num = 0;
	pBase = &_arr[0];
	for(;;) {
		int lenSub = (int)::wcscspn(pBase, delimiters);
		if(lenSub) (*pBuf)[num++].copyFrom(pBase, lenSub);
		if(pBase[lenSub] == L'\0') break;
		pBase += lenSub + 1;
	}
}

int __cdecl String::Sort(const void *a, const void *b)
{
	return ::lstrcmpi( ((const String*)a)->str(), ((const String*)b)->str() ); // callback to qsort()
}
