
#pragma once
#include <string>

class FileText final {
public:
	enum class Encoding { UNKNOWN, ASCII, UTF8, UTF16BE, UTF16LE, UTF32BE, UTF32LE, SCSU, BOCU1 };
	
	static std::pair<Encoding, int> getEncoding(const wchar_t *path, std::wstring *pErr = nullptr);
	static std::pair<Encoding, int> getEncoding(const std::wstring& path, std::wstring *pErr = nullptr);
	static bool                     read(std::wstring& buf, const wchar_t *path, std::wstring *pErr = nullptr);
	static bool                     read(std::wstring& buf, const std::wstring& path, std::wstring *pErr = nullptr);
	static bool                     writeUtf8(const std::wstring& text, const wchar_t *path, std::wstring *pErr = nullptr);
	static bool                     writeUtf8(const std::wstring& text, const std::wstring& path, std::wstring *pErr = nullptr);
};