//
// Ordinary string implementation, ever growing, with buffer capabilities.
// Part of TOOLOW - Thin Object Oriented Layer Over Win32.
// @author Rodrigo Cesar de Freitas Dias
// @see https://github.com/rodrigocfd/toolow
//

#pragma once
#include "Array.h"
#include <Windows.h>

class String;
void dbg(const wchar_t *fmt, ...);
void dbg(const String& s);

class String final {
private:
	Array<wchar_t> _arr;
public:
	friend void dbg(const wchar_t*, ...);

	String()                    { }
	String(const wchar_t *s)    { operator=(s); }
	String(const String& other) : _arr(other._arr) { }
	String(String&& other)      : _arr(MOVE(other._arr)) { }

	int            len() const                     { return _arr.size() ? ::lstrlen(&_arr[0]) : 0; }
	const wchar_t* str() const                     { return _arr.size() ? &_arr[0] : L""; }
	bool           isEmpty() const                 { return !_arr.size() || _arr[0] == L'\0'; }
	const wchar_t& operator[](int index) const     { return _arr[index]; }
	wchar_t&       operator[](int index)           { return _arr[index]; }
	const wchar_t* ptrAt(int index) const          { return &_arr[index]; }
	wchar_t*       ptrAt(int index)                { return &_arr[index]; }
	String&        operator=(const String& other)  { return operator=(other.str()); }
	String&        operator=(String&& other)       { _arr = MOVE(other._arr); return *this; }
	String&        operator=(const wchar_t *s);
	String&        reserve(int numCharsWithoutNull);
	int            reserved() const                { return _arr.size() - 1; }
	String&        append(const String& s)         { return append(s.str()); }
	String&        append(const wchar_t *s);
	String&        append(initializer_list<const wchar_t*> arr);
	String&        append(wchar_t ch);
	String&        insert(int at, const String& s) { return insert(at, s.str()); }
	String&        insert(int at, const wchar_t *s);
	String         substr(int start) const;
	String         substr(int start, int length) const;
	String&        copyFrom(const wchar_t *src, int numChars);
	String&        appendFrom(const wchar_t *src, int numChars);
	String&        trim();
	String&        toUpper() { if(!isEmpty()) _ChangeCase(&_arr[0], true); return *this; }
	String&        toLower() { if(!isEmpty()) _ChangeCase(&_arr[0], false); return *this; }
	String&        removeDiacritics();
	bool           equalsCS(const wchar_t *s) const    { return (isEmpty() && !s) ? true : CompareCS(&_arr[0], s) == 0; }
	bool           equalsCS(const String& other) const { return equalsCS(other.str()); }
	bool           equalsCI(const wchar_t *s) const    { return (isEmpty() && !s) ? true : CompareCI(&_arr[0], s) == 0; }
	bool           equalsCI(const String& other) const { return equalsCI(other.str()); }
	bool           beginsWithCS(const wchar_t *s) const    { return _beginsWith(s, true); }
	bool           beginsWithCS(const String& other) const { return beginsWithCS(other.str()); }
	bool           beginsWithCI(const wchar_t *s) const    { return _beginsWith(s, false); }
	bool           beginsWithCI(const String& other) const { return beginsWithCI(other.str()); }
	bool           endsWithCS(wchar_t ch) const;
	bool           endsWithCS(const wchar_t *s) const    { return _endsWith(s, true); }
	bool           endsWithCS(const String& other) const { return endsWithCS(other.str()); }
	bool           endsWithCI(const wchar_t *s) const    { return _endsWith(s, false); }
	bool           endsWithCI(const String& other) const { return endsWithCI(other.str()); }
	bool           isInt() const;
	bool           isFloat() const;
	int            toInt() const   { return isInt() ? ::_wtoi(&_arr[0]) : 0; }
	double         toFloat() const { return isFloat() ? ::_wtof(&_arr[0]) : 0; }
	int            findCS(wchar_t ch) const;
	int            findCS(const wchar_t *substring) const { return isEmpty() ? -1 : (int)(_Find(&_arr[0], substring, true) - &_arr[0]); }
	int            findCI(const wchar_t *substring) const { return isEmpty() ? -1 : (int)(_Find(&_arr[0], substring, false) - &_arr[0]); }
	int            findrCS(wchar_t ch) const;
	String&        replaceCS(const wchar_t *target, const wchar_t *replacement) { return _replace(target, replacement, true); }
	String&        replaceCI(const wchar_t *target, const wchar_t *replacement) { return _replace(target, replacement, false); }
	String&        invert();
	Array<String>  explode(const wchar_t *delimiters) const;

	static String        Fmt(const wchar_t *fmt, ...);
	static int           CompareCS(const wchar_t *a, const wchar_t *b, int nChars=0) { return _Compare(a, b, true, nChars); }
	static int           CompareCI(const wchar_t *a, const wchar_t *b, int nChars=0) { return _Compare(a, b, false, nChars); }
	static String        ParseUtf8(const BYTE *data, int length);
	static String        ParseUtf8(const Array<BYTE>& data) { return data.size() ? ParseUtf8(&data[0], data.size()) : L""; }
	static Array<String> ExplodeQuoted(const wchar_t *quotedStr);
	static Array<String> ExplodeMulti(const wchar_t *multiStr);
private:
	bool    _beginsWith(const wchar_t *s, bool isCS) const;
	bool    _endsWith(const wchar_t *s, bool isCS) const;
	String& _replace(const wchar_t *target, const wchar_t *replacement, bool isCS);

	static String   _Formatv(const wchar_t *fmt, va_list args);
	static wchar_t  _ChangeCase(wchar_t ch, bool toUpper);
	static wchar_t* _ChangeCase(wchar_t *txt, bool toUpper);
	static int      _Compare(const wchar_t *a, const wchar_t *b, bool isCS, int numCharsToSee=0);
	static const wchar_t* _Find(const wchar_t *full, const wchar_t *what, bool isCS);
};