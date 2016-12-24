/**
 * Part of WET - WinAPI Easy Toolkit
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/wet
 */

#pragma once
#include <string>
#include <vector>
#include <Windows.h>

namespace wet {

struct str final {
	static std::wstring format(const wchar_t* format, ...) {
		va_list args;
		va_start(args, format);
		int newLen = _vscwprintf(format, args); // calculate length, without terminating null
		std::wstring ret(newLen, L'\0');
#pragma warning (disable: 4996)
		_vsnwprintf(&ret[0], newLen, format, args); // do the job
#pragma warning (default: 4996)
		va_end(args);
		return ret;
	}

	static std::wstring& trim_nulls(std::wstring& s) {
		// When a std::wstring is initialized with any length, possibly to be used as a buffer,
		// the string length may not match the size() method, after the operation.
		// This function fixes this.
		if (!s.empty()) {
			s.resize( lstrlenW(s.c_str()) );
		}
		return s;
	}

	static std::wstring& trim(std::wstring& s) {
		if (s.empty()) return s;
		trim_nulls(s);

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

		std::copy(s.begin() + iFirst, // move the non-space chars back
			s.begin() + iLast + 1, s.begin());
		s.resize(iLast - iFirst + 1); // trim container size
		return s;
	}

	static std::wstring upper(const std::wstring& s) {
		std::wstring ret(s);
		CharUpperBuffW(&ret[0], static_cast<DWORD>(ret.size()));
		return ret;
	}

	static std::wstring lower(const std::wstring& s) {
		std::wstring ret(s);
		CharLowerBuffW(&ret[0], static_cast<DWORD>(ret.size()));
		return ret;
	}

	static std::wstring& remove_diacritics(std::wstring& s) {
		// Simple diacritics removal.
		const wchar_t* diacritics   = L"¡·¿‡√„¬‚ƒ‰…È»Ë ÍÀÎÕÌÃÏŒÓœÔ”Û“Ú’ı‘Ù÷ˆ⁄˙Ÿ˘€˚‹¸«Á≈Â–—Òÿ¯›˝";
		const wchar_t* replacements = L"AaAaAaAaAaEeEeEeEeIiIiIiIiOoOoOoOoOoUuUuUuUuCcAaDdNnOoYy";
		for (wchar_t& ch : s) {
			const wchar_t* pDiac = diacritics;
			const wchar_t* pRepl = replacements;
			while (*pDiac) {
				if (ch == *pDiac) ch = *pRepl; // in-place replacement
				++pDiac;
				++pRepl;
			}
		}
		return s;
	}

	static bool eqi(const std::wstring& s, const wchar_t* what) {
		return !lstrcmpiW(s.c_str(), what); // eq() is just operator==()
	}

	static bool eqi(const std::wstring& s, const std::wstring& what) {
		return eqi(s.c_str(), what.c_str());
	}

	static bool ends_with(const std::wstring& s, const wchar_t* what) {
		size_t whatLen = 0;
		if (!_first_ends_begins_check(s, what, whatLen)) {
			return false;
		}
		return !lstrcmpW(s.c_str() + s.size() - whatLen, what);
	}

	static bool ends_withi(const std::wstring& s, const wchar_t* what) {
		size_t whatLen = 0;
		if (!_first_ends_begins_check(s, what, whatLen)) {
			return false;
		}
		return !lstrcmpiW(s.c_str() + s.size() - whatLen, what);
	}

	static bool begins_with(const std::wstring& s, const wchar_t* what) {
		size_t whatLen = 0;
		if (!_first_ends_begins_check(s, what, whatLen)) {
			return false;
		}
		return !wcsncmp(s.c_str(), what, whatLen);
	}

	static bool begins_withi(const std::wstring& s, const wchar_t* what) {
		size_t whatLen = 0;
		if (!_first_ends_begins_check(s, what, whatLen)) {
			return false;
		}
		return !_wcsnicmp(s.c_str(), what, whatLen);
	}

	static size_t findi(const std::wstring& s, const wchar_t* what, size_t offset = 0) {
		std::wstring s2 = upper(s);
		std::wstring what2(what);
		CharUpperBuffW(&what2[0], static_cast<DWORD>(what2.size()));
		return s2.find(what2, offset);
	}

	static size_t rfindi(const std::wstring& s, const wchar_t* what, size_t offset = 0) {
		std::wstring s2 = upper(s);
		std::wstring what2(what);
		CharUpperBuffW(&what2[0], static_cast<DWORD>(what2.size()));
		return s2.rfind(what2, offset);
	}

	static std::wstring& replace(std::wstring& haystack, const wchar_t* needle, const wchar_t* replacement) {
		if (haystack.empty()) return haystack;

		size_t needleLen = lstrlenW(needle);
		if (!needleLen) return haystack;

		size_t replacementLen = lstrlenW(replacement);
		std::wstring output;
		size_t base = 0;
		size_t found = 0;

		for (;;) {
			found = haystack.find(needle, found);
			output.insert(output.size(), haystack, base, found - base);
			if (found != std::wstring::npos) {
				output.append(replacement);
				base = found = found + needleLen;
			} else {
				break;
			}
		}

		haystack.swap(output); // behaves like an in-place operation
		return haystack;
	}

	static std::wstring& replacei(std::wstring& haystack, const wchar_t* needle, const wchar_t* replacement) {
		if (!haystack.size()) return haystack;

		size_t needleLen = lstrlenW(needle);
		if (!needleLen) return haystack;

		std::wstring haystackU = upper(haystack);
		std::wstring needleU = upper(needle);

		size_t replacementLen = lstrlenW(replacement);
		std::wstring output;
		size_t base = 0;
		size_t found = 0;

		for (;;) {
			found = haystackU.find(needleU, found);
			output.insert(output.size(), haystack, base, found - base);
			if (found != std::wstring::npos) {
				output.append(replacement);
				base = found = found + needleLen;
			} else {
				break;
			}
		}

		haystack.swap(output); // behaves like an in-place operation
		return haystack;
	}

	static bool is_int(const std::wstring& s) {
		if (s.empty()) return false;
		if (s[0] != L'-' && !iswdigit(s[0]) && !iswblank(s[0])) return false;
		for (wchar_t ch : s) {
			if (!iswdigit(ch) && !iswblank(ch)) return false;
		}
		return true;
	}

	static bool is_uint(const std::wstring& s) {
		if (s.empty()) return false;
		for (wchar_t ch : s) {
			if (!iswdigit(ch) && !iswblank(ch)) return false;
		}
		return true;
	}

	static bool is_hex(const std::wstring& s) {
		if (s.empty()) return false;
		for (wchar_t ch : s) {
			if (!iswxdigit(ch) && !iswblank(ch)) return false;
		}
		return true;
	}

	static bool is_float(const std::wstring& s) {
		if (s.empty()) return false;
		if (s[0] != L'-' && s[0] != L'.' && !iswdigit(s[0]) && !iswblank(s[0])) return false;

		bool hasDot = false;
		for (wchar_t ch : s) {
			if (ch == L'.') {
				if (hasDot) {
					return false;
				} else {
					hasDot = true;
				}
			} else {
				if (!iswdigit(ch) && !iswblank(ch)) return false;
			}
		}
		return true;
	}

	static std::wstring to_str_with_separator(int n, wchar_t separator = L',') {
		std::wstring ret;
		ret.reserve(16);

		int abso = abs(n);
		BYTE blocks = 0;
		while (abso >= 1000) {
			abso = (abso - (abso % 1000)) / 1000;
			++blocks;
		}

		abso = abs(n);
		bool firstPass = true;
		do {
			int num = abso % 1000;
			wchar_t buf[8] = { 0 };

			if (blocks) {
				if (num < 100) lstrcatW(buf, L"0");
				if (num < 10) lstrcatW(buf, L"0");
			}

#pragma warning (disable: 4996)
			_itow(num, buf + lstrlenW(buf), 10);
#pragma warning (default: 4996)

			if (firstPass) {
				firstPass = false;
			} else {
				ret.insert(0, 1, separator);
			}

			ret.insert(0, buf);
			abso = (abso - (abso % 1000)) / 1000;
		} while (blocks--);

		if (n < 0) ret.insert(0, 1, L'-'); // prepend minus signal
		return ret;
	}

	static std::wstring to_str_with_separator(size_t n, wchar_t separator = L',') {
		return to_str_with_separator(static_cast<int>(n), separator);
	}

	static std::vector<std::wstring> explode(const std::wstring& s, const wchar_t* delimiter) {
		std::vector<std::wstring> ret;
		if (s.empty() || !delimiter) return ret;

		size_t base = 0, head = 0;

		for (;;) {
			head = s.find(delimiter, head);
			if (head == std::wstring::npos) break;
			ret.emplace_back();
			ret.back().insert(0, s, base, head - base);
			head += lstrlenW(delimiter);
			base = head;
		}

		ret.emplace_back();
		ret.back().insert(0, s, base, s.size() - base);
		return ret;
	}

	static std::vector<std::wstring> explode(const std::wstring& s, const std::wstring& delimiter) {
		return explode(s, delimiter.c_str());
	}

	static std::vector<std::wstring> explode_multi_zero(const wchar_t* s) {
		// Example multi-zero string:
		// L"first one\0second one\0third one\0"
		// Assumes a well-formed multiStr, which ends with two nulls.

		// Count number of null-delimited strings; string end with double null.
		size_t numStrings = 0;
		const wchar_t* pRun = s;
		while (*pRun) {
			++numStrings;
			pRun += lstrlenW(pRun) + 1;
		}

		// Alloc return array of strings.
		std::vector<std::wstring> ret;
		ret.reserve(numStrings);

		// Copy each string.
		pRun = s;
		for (size_t i = 0; i < numStrings; ++i) {
			ret.emplace_back(pRun);
			pRun += lstrlenW(pRun) + 1;
		}
		return ret;
	}

	static std::vector<std::wstring> explode_quoted(const wchar_t* s) {
		// Example quoted string:
		// "First one" NoQuoteSecond "Third one"

		// Count number of strings.
		size_t numStrings = 0;
		const wchar_t* pRun = s;
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
		std::vector<std::wstring> ret;
		ret.reserve(numStrings);

		// Alloc and copy each string.
		pRun = s;
		const wchar_t* pBase;
		int i = 0;
		while (*pRun) {
			if (*pRun == L'\"') { // begin of quoted string
				++pRun; // point to 1st char of string
				pBase = pRun;
				for (;;) {
					if (!*pRun) {
						break; // won't compute open-quoted
					} else if (*pRun == L'\"') {
						ret.emplace_back();
						ret.back().insert(0, pBase, pRun - pBase); // copy to buffer
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

				ret.emplace_back();
				ret.back().insert(0, pBase, pRun - pBase); // copy to buffer
				++i; // next string
			} else {
				++pRun; // some white space
			}
		}

		return ret;
	}

	enum class encoding { UNKNOWN, ASCII, WIN1252, UTF8, UTF16BE, UTF16LE, UTF32BE, UTF32LE, SCSU, BOCU1 };
	struct encoding_info final {
		encoding encType;
		size_t   bomSize;
	};

	static encoding_info get_encoding(const BYTE* data, size_t sz) {
		auto match = [&](const BYTE* pBom, int szBom)->bool {
			return (sz >= static_cast<size_t>(szBom)) &&
				!memcmp(data, pBom, sizeof(BYTE) * szBom);
		};

		// https://en.wikipedia.org/wiki/Byte_order_mark

		BYTE utf8[] = { 0xEF, 0xBB, 0xBF }; // UTF-8 BOM
		if (match(utf8, 3)) return { encoding::UTF8, 3 }; // BOM size in bytes

		BYTE utf16be[] = { 0xFE, 0xFF };
		if (match(utf16be, 2)) return { encoding::UTF16BE, 2 };

		BYTE utf16le[] = { 0xFF, 0xFE };
		if (match(utf16le, 2)) return { encoding::UTF16LE, 2 };

		BYTE utf32be[] = { 0x00, 0x00, 0xFE, 0xFF };
		if (match(utf32be, 4)) return { encoding::UTF32BE, 4 };

		BYTE utf32le[] = { 0xFF, 0xFE, 0x00, 0x00 };
		if (match(utf32le, 4)) return { encoding::UTF32LE, 4 };

		BYTE scsu[] = { 0x0E, 0xFE, 0xFF };
		if (match(scsu, 3)) return { encoding::SCSU, 3 };

		BYTE bocu1[] = { 0xFB, 0xEE, 0x28 };
		if (match(bocu1, 3)) return { encoding::BOCU1, 3 };

		// No BOM found, guess UTF-8 without BOM, or Windows-1252 (superset of ISO-8859-1).
		bool canBeWin1252 = false;
		for (size_t i = 0; i < sz; ++i) {
			if (data[i] > 0x7F) { // 127
				canBeWin1252 = true;
				if ( i <= sz - 2 && (
					(data[i] == 0xC2 && (data[i+1] >= 0xA1 && data[i+1] <= 0xBF)) || // http://www.utf8-chartable.de
					(data[i] == 0xC3 && (data[i+1] >= 0x80 && data[i+1] <= 0xBF)) ))
				{
					return { encoding::UTF8, 0 }; // UTF-8 without no BOM
				}
			}
		}
		return { (canBeWin1252 ? encoding::WIN1252 : encoding::ASCII), 0 };
	}

	static encoding_info get_encoding(const std::vector<BYTE>& data) {
		return get_encoding(data.data(), data.size());
	}

	static const wchar_t* get_linebreak(const std::wstring& s) {
		for (size_t i = 0; i < s.size() - 1; ++i) {
			if (s[i] == L'\r') {
				return s[i + 1] == L'\n' ? L"\r\n" : L"\r";
			} else if (s[i] == L'\n') {
				return s[i + 1] == L'\r' ? L"\n\r" : L"\n";
			}
		}
		return nullptr; // unknown
	}

	static std::vector<BYTE> to_utf8_blob(const std::wstring& s, bool writeBom) {
		std::vector<BYTE> ret;
		if (!s.empty()) {
			BYTE utf8bom[] = { 0xEF, 0xBB, 0xBF };
			int szBom = writeBom ? ARRAYSIZE(utf8bom) : 0;

			int neededLen = WideCharToMultiByte(CP_UTF8, 0,
				s.c_str(), static_cast<int>(s.size()),
				nullptr, 0, nullptr, 0);
			ret.resize(neededLen + szBom);

			if (writeBom) memcpy(&ret[0], utf8bom, szBom);
			WideCharToMultiByte(CP_UTF8, 0,
				s.c_str(), static_cast<int>(s.size()), 
				reinterpret_cast<char*>(&ret[0 + szBom]),
				neededLen, nullptr, nullptr);
		}
		return ret;
	}

	static bool parse_blob(const BYTE* data, size_t sz, std::wstring& buf, std::wstring* pErr = nullptr) {
		if (!data || !sz) return true;
		encoding_info fileEnc = get_encoding(data, sz);
		data += fileEnc.bomSize; // skip BOM, if any

		auto happy = [&]()->bool {
			if (pErr) pErr->clear();
			return true;
		};
		auto fail = [&](const wchar_t* errMsg)->bool {
			if (pErr) *pErr = errMsg;
			return false;
		};

		switch (fileEnc.encType) {
		case encoding::UNKNOWN:
		case encoding::ASCII:   buf = _parse_ascii(data, sz); return happy();
		case encoding::WIN1252: buf = _parse_encoded(data, sz, 1252); return happy();
		case encoding::UTF8:    buf = _parse_encoded(data, sz, CP_UTF8); return happy();
		case encoding::UTF16BE: return fail(L"UTF-16 big endian: encoding not implemented.");
		case encoding::UTF16LE: return fail(L"UTF-16 little endian: encoding not implemented.");
		case encoding::UTF32BE: return fail(L"UTF-32 big endian: encoding not implemented.");
		case encoding::UTF32LE: return fail(L"UTF-32 little endian: encoding not implemented.");
		case encoding::SCSU:    return fail(L"Standard compression scheme for Unicode: encoding not implemented.");
		case encoding::BOCU1:   return fail(L"Binary ordered compression for Unicode: encoding not implemented.");
		}

		return fail(L"Failed to parse: unknown encoding.");
	}

	static bool parse_blob(const std::vector<BYTE>& data, std::wstring& buf, std::wstring* pErr = nullptr) {
		return parse_blob(data.data(), data.size(), buf, pErr);
	}

private:
	static bool _first_ends_begins_check(const std::wstring& s, const wchar_t* what, size_t& whatLen) {
		if (s.empty()) return false;

		whatLen = lstrlenW(what);
		if (!whatLen || whatLen > s.size()) {
			return false;
		}
		return true;
	}

	static std::wstring _parse_ascii(const BYTE* data, size_t sz) {
		std::wstring ret;
		if (data && sz) {
			ret.resize(sz);
			for (size_t i = 0; i < sz; ++i) {
				ret[i] = static_cast<wchar_t>(data[i]); // raw conversion
			}
			trim_nulls(ret);
		}
		return ret;
	}

	static std::wstring _parse_encoded(const BYTE* data, size_t sz, UINT codePage) {
		std::wstring ret;
		if (data && sz) {
			int neededLen = MultiByteToWideChar(codePage, 0, reinterpret_cast<const char*>(data),
				static_cast<int>(sz), nullptr, 0);
			ret.resize(neededLen);
			MultiByteToWideChar(codePage, 0, reinterpret_cast<const char*>(data),
				static_cast<int>(sz), &ret[0], neededLen);
			trim_nulls(ret);
		}
		return ret;
	}
};

}//namespace wet