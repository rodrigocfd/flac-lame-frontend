/*!
 * @file
 * @brief String and vector utilities.
 * @details Part of WOLF - Win32 Object Lambda Framework.
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/wolf
 */

#pragma once
#include <string>
#include <vector>
#include <Windows.h>

namespace wolf {

namespace str {
	enum class Sens { YES, NO };

	void Dbg(const wchar_t *fmt, ...);
	inline std::wstring& TrimNulls(std::wstring& s) { if (!s.empty()) s.resize(::lstrlen(s.c_str())); return s; }
	std::wstring  Sprintf(const wchar_t *fmt, ...);
	std::wstring& Trim(std::wstring& s);
	std::wstring& ToUpper(std::wstring& s);
	std::wstring& ToLower(std::wstring& s);
	bool IsInt(const std::wstring& s);
	bool IsFloat(const std::wstring& s);
	int         LexCmp(Sens se, const wchar_t *a, const wchar_t *b, size_t nChars=0);
	inline int  LexCmp(Sens se, const std::wstring& a, const wchar_t *b, size_t nChars=0)      { return LexCmp(se, a.c_str(), b, nChars); }
	inline int  LexCmp(Sens se, const std::wstring& a, const std::wstring& b, size_t nChars=0) { return LexCmp(se, a.c_str(), b.c_str(), nChars); }
	inline bool Cmp(Sens se, const std::wstring& a, const wchar_t *b)      { return !LexCmp(se, a, b); }
	inline bool Cmp(Sens se, const std::wstring& a, const std::wstring& b) { return !LexCmp(se, a, b); }
	bool BeginsWith(Sens se, const std::wstring& s, const wchar_t *what);
	bool EndsWith(Sens se, const std::wstring& s, const wchar_t *what);
	int            Find(Sens se, const std::wstring& s, wchar_t what);
	const wchar_t* Find(Sens se, const wchar_t *s, const wchar_t *what);
	int            Find(Sens se, const std::wstring& s, const wchar_t *what);
	inline int     Find(Sens se, const std::wstring& s, const std::wstring& what) { return Find(se, s, what.c_str()); }
	int            FindRev(Sens se, const std::wstring& s, wchar_t what);
	const wchar_t* FindRev(Sens se, const wchar_t *s, const wchar_t *what);
	int            FindRev(Sens se, const std::wstring& s, const wchar_t *what);
	inline int     FindRev(Sens se, const std::wstring& s, const std::wstring& what) { return FindRev(se, s, what.c_str()); }
	std::wstring&        Replace(Sens se, std::wstring& s, const wchar_t *target, const wchar_t *replacement);
	inline std::wstring& Replace(Sens se, std::wstring& s, const std::wstring& target, const wchar_t *replacement)      { return Replace(se, s, target.c_str(), replacement); }
	inline std::wstring& Replace(Sens se, std::wstring& s, const wchar_t *target, const std::wstring& replacement)      { return Replace(se, s, target, replacement.c_str()); }
	inline std::wstring& Replace(Sens se, std::wstring& s, const std::wstring& target, const std::wstring& replacement) { return Replace(se, s, target.c_str(), replacement.c_str()); }
	std::wstring&       RemoveDiacritics(std::wstring& s);
	std::wstring        ParseUtf8(const BYTE *data, size_t length);
	inline std::wstring ParseUtf8(const std::vector<BYTE>& data) { return ParseUtf8(&data[0], data.size()); }
	std::vector<std::wstring> Explode(const std::wstring& s, const wchar_t *delimiters);
	std::vector<std::wstring> ExplodeMultiStr(const wchar_t *multiStr);
}//namespace str


namespace vec {
	template<typename T> inline void Append(std::vector<T>& v, const std::vector<T>& other) { v.insert(v.end(), other.begin(), other.end()); }
	template<typename T> inline void Remove(std::vector<T>& v, int idx)                     { v.erase(std::next(v.begin(), idx)); }
	template<typename T> inline void Remove(std::vector<T>& v, int idxFirst, int idxLast)   { v.erase(std::next(v.begin(), idxFirst), std::next(v.begin(), idxLast + 1)); }
	template<typename T> void ReposElem(std::vector<T>& v, size_t idx, size_t toIdx) {
		if (idx == toIdx || idx >= v.size() || toIdx >= v.size()) return;
		std::swap(v[idx], v[toIdx]);
		if (idx < toIdx) { for (size_t i = idx; i <= toIdx - 2; ++i) std::swap(v[i], v[i + 1]); }
		else { for (size_t i = idx; i >= toIdx + 2; --i) std::swap(v[i], v[i - 1]); }
	}
}//namespace vec

}//namespace wolf
