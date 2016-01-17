
#pragma once
#include <string>
#include <vector>
#include <Windows.h>

struct Str final {
	static std::wstring  format(const wchar_t *format, ...);
	static std::wstring& trimNulls(std::wstring& s);
	static std::wstring& trim(std::wstring& s);
	static std::wstring& upper(std::wstring& s);
	static std::wstring& lower(std::wstring& s);
	static std::wstring& removeDiacritics(std::wstring& s);
	static std::wstring  parseUtf8(const BYTE *data, size_t length);
	static std::wstring  parseUtf8(const std::vector<BYTE>& data);
	static std::wstring  folderFromPath(const std::wstring& path);
	static std::wstring  fileFromPath(const std::wstring& path);
	static size_t        find(const std::wstring& s, const wchar_t *what, size_t offset = 0);
	static size_t        findI(const std::wstring& s, const wchar_t *what, size_t offset = 0);
	static size_t        findLast(const std::wstring& s, const wchar_t *what, size_t offset = std::wstring::npos);
	static size_t        findLastI(const std::wstring& s, const wchar_t *what, size_t offset = std::wstring::npos);
	static std::wstring& replace(std::wstring& s, const wchar_t *what, const wchar_t *replacement);
	static std::wstring& replaceI(std::wstring& s, const wchar_t *what, const wchar_t *replacement);
	static bool eq(const std::wstring& s, const wchar_t *what);
	static bool eq(const std::wstring& s, const std::wstring& what);
	static bool eqI(const std::wstring& s, const wchar_t *what);
	static bool eqI(const std::wstring& s, const std::wstring& what);
	static bool endsWith(const std::wstring& s, const wchar_t *what);
	static bool endsWith(const std::wstring& s, const std::wstring& what);
	static bool endsWithI(const std::wstring& s, const wchar_t *what);
	static bool endsWithI(const std::wstring& s, const std::wstring& what);
	static bool beginsWith(const std::wstring& s, const wchar_t *what);
	static bool beginsWith(const std::wstring& s, const std::wstring& what);
	static bool beginsWithI(const std::wstring& s, const wchar_t *what);
	static bool beginsWithI(const std::wstring& s, const std::wstring& what);
	static bool isFloat(const std::wstring& s, float *pNum = nullptr);
	static bool isFloatU(const std::wstring& s, float *pNum = nullptr);
	static bool isDouble(const std::wstring& s, double *pNum = nullptr);
	static bool isDoubleU(const std::wstring& s, double *pNum = nullptr);
	static bool isInt(const std::wstring& s, int *pNum = nullptr);
	static bool isIntU(const std::wstring& s, unsigned int *pNum = nullptr);
	static bool isHex(const std::wstring& s, int *pNum = nullptr);
	static bool isHexU(const std::wstring& s, unsigned int *pNum = nullptr);
	static std::vector<std::wstring> explode(const wchar_t *s, const wchar_t *delimiters);
	static std::vector<std::wstring> explode(const std::wstring& s, const wchar_t *delimiters);

	enum class Encoding { ASCII, UTF8, UTF16BE, UTF16LE, UTF32BE, UTF32LE, SCSU, BOCU1 };
	static Encoding getEncoding(const BYTE *pData, size_t sz);
	static Encoding getEncoding(const std::vector<BYTE>& data);
};