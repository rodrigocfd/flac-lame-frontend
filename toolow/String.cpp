
#pragma warning(disable:4996) // _vsnwprintf, wcsncpy
#include <stdio.h>
#include "String.h"

#define STR_ALLOC_STACK_OR_HEAP(ptr, nchars) \
	(ptr) = ((nchars) * sizeof(wchar_t) > 1024 * 2) ? \
		(wchar_t*)::malloc(sizeof(wchar_t) * (nchars)) : \
		(wchar_t*)::_alloca(sizeof(wchar_t) * (nchars))

#define STR_FREE_STACK_OR_HEAP(ptr, nchars) \
	if((nchars) * sizeof(wchar_t) > 1024 * 2) \
		::free(ptr)

String& String::operator=(const wchar_t *s)
{
	int newLen = ::lstrlen(s);
	if(!_arr.size() && !newLen) return *this; // still unallocated and no new string: do nothing
	this->reserve(newLen); // also places terminating null
	::memcpy(&_arr[0], s, sizeof(wchar_t) * newLen);
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
	::memcpy(&_arr[ourLen], s, sizeof(wchar_t) * theirLen);
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

String& String::getSubstrFrom(const wchar_t *src, int start, int length)
{
	int hisLen = ::lstrlen(src);
	if(hisLen && start <= hisLen - 1) {
		if(start < 0) start = 0;
		if(length <= 0) length = hisLen - start + length; // negative length acts as backwards index
		if(length > 0) {
			if(start + length > hisLen - 1) length = hisLen - start;

			this->reserve(length);
			for(int i = 0; i < length; ++i)
				_arr[i] = src[i + start];
		}
	}
	return *this;
}

bool String::equals(const wchar_t *s, String::Case c, String::Diacritics d) const
{
	wchar_t *us = NULL,           *them = NULL;
	int      ourLen = this->len(), theirLen = ::lstrlen(s);

	if(d == IGNOREDIACR) { // duplicate both strings to remove the diacritics
		STR_ALLOC_STACK_OR_HEAP(us, ourLen + 1);
		::memcpy(us, &_arr[0], sizeof(wchar_t) * (ourLen + 1));
		_RemDiacr(us);

		STR_ALLOC_STACK_OR_HEAP(them, theirLen + 1);
		::memcpy(them, s, sizeof(wchar_t) * (theirLen + 1));
		_RemDiacr(them);
	} else {
		us = (wchar_t*)this->str();
		them = (wchar_t*)s;
	}

	bool bRet = (c == INSENS) ? !::lstrcmpi(us, them) : !::lstrcmp(us, them);

	if(d == IGNOREDIACR) {
		STR_FREE_STACK_OR_HEAP(us, ourLen + 1);
		STR_FREE_STACK_OR_HEAP(them, theirLen + 1);
	}

	return bRet;
}

bool String::startsWith(const wchar_t *s, String::Case c, String::Diacritics d) const
{
	wchar_t *us = NULL,           *them = NULL;
	int      ourLen = this->len(), theirLen = ::lstrlen(s);

	if(!ourLen || theirLen > ourLen) return false;

	STR_ALLOC_STACK_OR_HEAP(us, theirLen + 1);
	::memcpy(us, &_arr[0], sizeof(wchar_t) * theirLen);
	us[theirLen] = L'\0';

	if(d == IGNOREDIACR) {
		_RemDiacr(us);

		STR_ALLOC_STACK_OR_HEAP(them, theirLen + 1);
		::memcpy(them, s, sizeof(wchar_t) * (theirLen + 1));
		_RemDiacr(them);
	} else {
		them = (wchar_t*)s;
	}

	bool bRet = (c == INSENS) ? !::lstrcmpi(us, them) : !::lstrcmp(us, them);

	STR_FREE_STACK_OR_HEAP(us, theirLen + 1);
	if(d == IGNOREDIACR)
		STR_FREE_STACK_OR_HEAP(them, theirLen + 1);

	return bRet;
}

bool String::endsWith(const wchar_t *s, String::Case c, String::Diacritics d) const
{
	wchar_t *us = NULL,           *them = NULL;
	int      ourLen = this->len(), theirLen = ::lstrlen(s);

	if(!ourLen || theirLen > ourLen) return false;

	if(d == IGNOREDIACR) {
		STR_ALLOC_STACK_OR_HEAP(us, theirLen + 1);
		::memcpy(us, &_arr[ourLen - theirLen], sizeof(wchar_t) * (theirLen + 1));
		_RemDiacr(us);

		STR_ALLOC_STACK_OR_HEAP(them, theirLen + 1);
		::memcpy(them, s, sizeof(wchar_t) * (theirLen + 1));
		_RemDiacr(them);
	} else {
		us = (wchar_t*)&_arr[ourLen - theirLen];
		them = (wchar_t*)s;
	}

	bool bRet = (c == INSENS) ? !::lstrcmpi(us, them) : !::lstrcmp(us, them);

	if(d == IGNOREDIACR) {
		STR_FREE_STACK_OR_HEAP(us, theirLen + 1);
		STR_FREE_STACK_OR_HEAP(them, theirLen + 1);
	}

	return bRet;
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
	wchar_t *us = NULL,           *them = NULL;
	int      ourLen = this->len(), theirLen = ::lstrlen(substring);

	if(!ourLen || theirLen > ourLen) return -1;

	if(c == INSENS || d == IGNOREDIACR) { // duplicate both strings to remove the diacritics and/or lowercase
		STR_ALLOC_STACK_OR_HEAP(us, ourLen + 1);
		::memcpy(us, &_arr[0], sizeof(wchar_t) * (ourLen + 1));
		if(c == INSENS) ::CharLower(us);
		if(d == IGNOREDIACR) _RemDiacr(us);

		STR_ALLOC_STACK_OR_HEAP(them, theirLen + 1);
		::memcpy(them, substring, sizeof(wchar_t) * (theirLen + 1));
		if(c == INSENS) ::CharLower(them);
		if(d == IGNOREDIACR) _RemDiacr(them);
	} else {
		us = (wchar_t*)this->str();
		them = (wchar_t*)substring;
	}

	int iRet = -1;
	wchar_t *p = ::wcsstr(us, them);
	if(p) iRet = (int)(p - us); // index of position

	if(c == INSENS || d == IGNOREDIACR) {
		STR_FREE_STACK_OR_HEAP(us, ourLen + 1);
		STR_FREE_STACK_OR_HEAP(them, theirLen + 1);
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
	wchar_t *targetNorm = NULL,            *usNorm = NULL;
	int      targetLen = ::lstrlen(target), usLen = this->len();

	if(c == INSENS || d == IGNOREDIACR) { // duplicate target and str() to remove diacritics and/or lowercase
		STR_ALLOC_STACK_OR_HEAP(targetNorm, targetLen + 1);
		::memcpy(targetNorm, target, sizeof(wchar_t) * (targetLen + 1));
		if(c == INSENS) ::CharLower(targetNorm);
		if(d == IGNOREDIACR) _RemDiacr(targetNorm);

		STR_ALLOC_STACK_OR_HEAP(usNorm, usLen + 1);
		::memcpy(usNorm, &_arr[0], sizeof(wchar_t) * (usLen + 1));
		if(c == INSENS) ::CharLower(usNorm);
		if(d == IGNOREDIACR) _RemDiacr(usNorm);
	} else {
		targetNorm = (wchar_t*)target;
		usNorm = (wchar_t*)this->str();
	}

	int replacementLen = ::lstrlen(replacement);

	// Count occurrences of target.
	int occurrences = 0;
	const wchar_t *p = usNorm;
	while(p = ::wcsstr(p, targetNorm)) {
		++occurrences;
		p += targetLen; // go beyond
	}

	// Alloc exact buffer to receive the replaced string.
	String buf;
	buf.reserve((replacementLen - targetLen) * occurrences + this->len());

	// Copy the string with the replacements made.
	wchar_t *pBuf = &buf[0];
	const wchar_t *base = usNorm, *orig = &_arr[0];
	p = usNorm;
	while(p = ::wcsstr(p, targetNorm)) {
		::memcpy(pBuf, orig, sizeof(wchar_t) * (int)(p - base)); // copy chars until before replacement
		pBuf += p - base;
		::memcpy(pBuf, replacement, sizeof(wchar_t) * replacementLen);
		pBuf += replacementLen;
		p += targetLen;
		base = p;
		orig += p - base;
	}
	::lstrcpy(pBuf, base); // copy rest of original string, including terminating null
	
	_arr.swap(&buf._arr); // now the new buf is us, our old buffer will be released

	if(c == INSENS || d == IGNOREDIACR) {
		STR_FREE_STACK_OR_HEAP(targetNorm, targetLen + 1);
		STR_FREE_STACK_OR_HEAP(usNorm, usLen + 1);
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

void String::explode(const wchar_t *delimiters, Array<String> *pBuf) const
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

String& String::fromUtf8(const BYTE *data, int length)
{
	int neededLen = ::MultiByteToWideChar(CP_UTF8, 0, (const char*)data, length, 0, 0);
	this->reserve(neededLen);
	::MultiByteToWideChar(CP_UTF8, 0, (const char*)data, length, &_arr[0], neededLen);
	return *this;
}

int __cdecl String::Sort(const void *a, const void *b)
{
	return ::lstrcmpi( ((const String*)a)->str(), ((const String*)b)->str() ); // callback to qsort()
}

void String::_RemDiacr(wchar_t *txt)
{
	const wchar_t *diacritics   = L"ÁáÀàÃãÂâÄäÉéÈèÊêËëÍíÌìÎîÏïÓóÒòÕõÔôÖöÚúÙùÛûÜüÇçÅåĞğÑñØøİı";
	const wchar_t *replacements = L"AaAaAaAaAaEeEeEeEeIiIiIiIiOoOoOoOoOoUuUuUuUuCcAaDdNnOoYy";

	for(int s = 0, length = ::lstrlen(txt); s < length; ++s)
		for(int d = 0; d < 56; ++d) // yes, hand-counted string length
			if(txt[s] == diacritics[d])
				txt[s] = replacements[d];
}