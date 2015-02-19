/*!
 * String utilities.
 * Part of OWL - Object Win32 Library.
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/wolf
 */

#pragma once
#include <string>
#include <vector>
#include <Windows.h>

namespace owl {

inline std::wstring& TrimNulls(std::wstring& s) { if (!s.empty()) s.resize(::lstrlen(s.c_str())); return s; }
std::wstring  Sprintf(const wchar_t *fmt, ...);
std::wstring& Trim(std::wstring& s);
std::wstring& ToUpper(std::wstring& s);
std::wstring& ToLower(std::wstring& s);
bool IsInt(const std::wstring& s);
bool IsFloat(const std::wstring& s);
int         StrLex(const wchar_t *a, const wchar_t *b, size_t nChars=0);
inline int  StrLex(const std::wstring& a, const wchar_t *b, size_t nChars=0)      { return StrLex(a.c_str(), b, nChars); }
inline int  StrLex(const std::wstring& a, const std::wstring& b, size_t nChars=0) { return StrLex(a.c_str(), b.c_str(), nChars); }
int         StrLexi(const wchar_t *a, const wchar_t *b, size_t nChars=0);
inline int  StrLexi(const std::wstring& a, const wchar_t *b, size_t nChars=0)      { return StrLexi(a.c_str(), b, nChars); }
inline int  StrLexi(const std::wstring& a, const std::wstring& b, size_t nChars=0) { return StrLexi(a.c_str(), b.c_str(), nChars); }
inline bool StrEq(const std::wstring& a, const wchar_t *b)      { return !StrLex(a, b); }
inline bool StrEq(const std::wstring& a, const std::wstring& b) { return !StrLex(a, b); }
inline bool StrEqi(const std::wstring& a, const wchar_t *b)      { return !StrLexi(a, b); }
inline bool StrEqi(const std::wstring& a, const std::wstring& b) { return !StrLexi(a, b); }
bool BeginsWith(const std::wstring& s, const wchar_t *what);
bool BeginsWithi(const std::wstring& s, const wchar_t *what);
bool EndsWith(const std::wstring& s, const wchar_t *what);
bool EndsWithi(const std::wstring& s, const wchar_t *what);
int            StrFind(const std::wstring& s, wchar_t what);
const wchar_t* StrFind(const wchar_t *s, const wchar_t *what);
int            StrFind(const std::wstring& s, const wchar_t *what);
inline int     StrFind(const std::wstring& s, const std::wstring& what) { return StrFind(s, what.c_str()); }
int            StrFindi(const std::wstring& s, wchar_t what);
const wchar_t* StrFindi(const wchar_t *s, const wchar_t *what);
int            StrFindi(const std::wstring& s, const wchar_t *what);
inline int     StrFindi(const std::wstring& s, const std::wstring& what) { return StrFindi(s, what.c_str()); }
int            StrRFind(const std::wstring& s, wchar_t what);
const wchar_t* StrRFind(const wchar_t *s, const wchar_t *what);
int            StrRFind(const std::wstring& s, const wchar_t *what);
inline int     StrRFind(const std::wstring& s, const std::wstring& what) { return StrRFind(s, what.c_str()); }
int            StrRFindi(const std::wstring& s, wchar_t what);
const wchar_t* StrRFindi(const wchar_t *s, const wchar_t *what);
int            StrRFindi(const std::wstring& s, const wchar_t *what);
inline int     StrRFindi(const std::wstring& s, const std::wstring& what) { return StrRFindi(s, what.c_str()); }
std::wstring&        StrReplace(std::wstring& s, const wchar_t *target, const wchar_t *replacement);
inline std::wstring& StrReplace(std::wstring& s, const std::wstring& target, const wchar_t *replacement)      { return StrReplace(s, target.c_str(), replacement); }
inline std::wstring& StrReplace(std::wstring& s, const wchar_t *target, const std::wstring& replacement)      { return StrReplace(s, target, replacement.c_str()); }
inline std::wstring& StrReplace(std::wstring& s, const std::wstring& target, const std::wstring& replacement) { return StrReplace(s, target.c_str(), replacement.c_str()); }
std::wstring&        StrReplacei(std::wstring& s, const wchar_t *target, const wchar_t *replacement);
inline std::wstring& StrReplacei(std::wstring& s, const std::wstring& target, const wchar_t *replacement)      { return StrReplacei(s, target.c_str(), replacement); }
inline std::wstring& StrReplacei(std::wstring& s, const wchar_t *target, const std::wstring& replacement)      { return StrReplacei(s, target, replacement.c_str()); }
inline std::wstring& StrReplacei(std::wstring& s, const std::wstring& target, const std::wstring& replacement) { return StrReplacei(s, target.c_str(), replacement.c_str()); }
std::wstring        ParseUtf8(const BYTE *data, size_t length);
inline std::wstring ParseUtf8(const std::vector<BYTE>& data) { return ParseUtf8(&data[0], data.size()); }
std::vector<std::wstring> Explode(const std::wstring& s, const wchar_t *delimiters);
std::vector<std::wstring> ExplodeMultiStr(const wchar_t *multiStr);

}//namespace owl