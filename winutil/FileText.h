
#pragma once
#include <string>
#include <vector>
#include <Windows.h>

class FileText final {
public:
	enum class Encoding { UNKNOWN, ASCII, UTF8, UTF16BE, UTF16LE, UTF32BE, UTF32LE, SCSU, BOCU1 };
	
	static std::pair<Encoding, int> getEncoding(const BYTE *src, size_t sz);
	static std::pair<Encoding, int> getEncoding(const std::vector<BYTE> src) { return getEncoding(src.data(), src.size()); }
	static std::pair<Encoding, int> getEncoding(const wchar_t *path, std::wstring *pErr = nullptr);
	static std::pair<Encoding, int> getEncoding(const std::wstring& path, std::wstring *pErr = nullptr) { return getEncoding(path.c_str(), pErr); }

	static const wchar_t* getLinebreak(const std::wstring& src);
	
	static bool read(std::wstring& buf, const BYTE *src, size_t sz, std::wstring *pErr = nullptr);
	static bool read(std::wstring& buf, const std::vector<BYTE> src, std::wstring *pErr = nullptr) { return read(buf, src.data(), src.size(), pErr); }
	static bool read(std::wstring& buf, const wchar_t *path, std::wstring *pErr = nullptr);
	static bool read(std::wstring& buf, const std::wstring& path, std::wstring *pErr = nullptr) { return read(buf, path.c_str(), pErr); }

	static bool readLines(std::vector<std::wstring>& buf, const BYTE *src, size_t sz, std::wstring *pErr = nullptr);
	static bool readLines(std::vector<std::wstring>& buf, const std::vector<BYTE> src, std::wstring *pErr = nullptr) { return readLines(buf, src.data(), src.size(), pErr); }
	static bool readLines(std::vector<std::wstring>& buf, const wchar_t *path, std::wstring *pErr = nullptr);
	static bool readLines(std::vector<std::wstring>& buf, const std::wstring& path, std::wstring *pErr = nullptr) { return readLines(buf, path.c_str(), pErr); }

	static bool writeUtf8(const std::wstring& text, const wchar_t *path, std::wstring *pErr = nullptr);
	static bool writeUtf8(const std::wstring& text, const std::wstring& path, std::wstring *pErr = nullptr) { return writeUtf8(text, path.c_str(), pErr); }
};