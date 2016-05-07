/**
 * Part of WinUtil - Windows Utilities
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winutil
 */

#pragma once
#include <string>
#include <vector>
#include <Windows.h>

namespace winutil {

class file_text final {
public:
	enum class encoding { UNKNOWN, ASCII, UTF8, UTF16BE, UTF16LE, UTF32BE, UTF32LE, SCSU, BOCU1 };	
	struct encoding_info final {
		encoding encType;
		size_t   bomSize;
	};
	
	static encoding_info get_encoding(const BYTE *src, size_t sz);
	static encoding_info get_encoding(const std::vector<BYTE> src) { return get_encoding(src.data(), src.size()); }
	static encoding_info get_encoding(const wchar_t *path, std::wstring *pErr = nullptr);
	static encoding_info get_encoding(const std::wstring& path, std::wstring *pErr = nullptr) { return get_encoding(path.c_str(), pErr); }

	static const wchar_t* get_linebreak(const std::wstring& src);
	
	static bool read(std::wstring& buf, const BYTE *src, size_t sz, std::wstring *pErr = nullptr);
	static bool read(std::wstring& buf, const std::vector<BYTE> src, std::wstring *pErr = nullptr) { return read(buf, src.data(), src.size(), pErr); }
	static bool read(std::wstring& buf, const wchar_t *path, std::wstring *pErr = nullptr);
	static bool read(std::wstring& buf, const std::wstring& path, std::wstring *pErr = nullptr)    { return read(buf, path.c_str(), pErr); }

	static bool read_lines(std::vector<std::wstring>& buf, const BYTE *src, size_t sz, std::wstring *pErr = nullptr);
	static bool read_lines(std::vector<std::wstring>& buf, const std::vector<BYTE> src, std::wstring *pErr = nullptr) { return read_lines(buf, src.data(), src.size(), pErr); }
	static bool read_lines(std::vector<std::wstring>& buf, const wchar_t *path, std::wstring *pErr = nullptr);
	static bool read_lines(std::vector<std::wstring>& buf, const std::wstring& path, std::wstring *pErr = nullptr)    { return read_lines(buf, path.c_str(), pErr); }

	static bool write_utf8(const std::wstring& text, const wchar_t *path, std::wstring *pErr = nullptr);
	static bool write_utf8(const std::wstring& text, const std::wstring& path, std::wstring *pErr = nullptr) { return write_utf8(text, path.c_str(), pErr); }
};

}//namespace winutil