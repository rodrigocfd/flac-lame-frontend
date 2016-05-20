/**
 * Part of WinUtil - Windows Utilities
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winutil
 */

#pragma once
#include <vector>
#include <Windows.h>

namespace winutil {

class file final {
public:
	enum class access { READONLY, READWRITE };
private:
	HANDLE _hFile;
	access _access;
public:
	~file() { close(); }
	file();
	file(file&& f);
	file& operator=(file&& f);

	HANDLE hfile() const      { return _hFile; }
	access get_access() const { return _access; }
	void   close();
	size_t size() const       { return GetFileSize(_hFile, nullptr); }
	bool   open(const std::wstring& filePath, access accessType, std::wstring* pErr = nullptr);
	bool   set_new_size(size_t newSize, std::wstring* pErr = nullptr);
	bool   truncate(std::wstring* pErr = nullptr)                                  { return set_new_size(0, pErr); }
	bool   get_content(std::vector<BYTE>& buf, std::wstring* pErr = nullptr) const;
	bool   write(const BYTE* pData, size_t sz, std::wstring* pErr = nullptr);
	bool   write(const std::vector<BYTE>& data, std::wstring* pErr = nullptr)      { return write(&data[0], data.size(), pErr); }
	bool   rewind(std::wstring* pErr = nullptr);

	static bool exists(const wchar_t* fileOrFolder)      { return GetFileAttributes(fileOrFolder) != INVALID_FILE_ATTRIBUTES; }
	static bool exists(const std::wstring& fileOrFolder) { return exists(fileOrFolder.c_str()); }
	static bool is_dir(const wchar_t* thePath)      { return (GetFileAttributes(thePath) & FILE_ATTRIBUTE_DIRECTORY) != 0; }
	static bool is_dir(const std::wstring& thePath) { return is_dir(thePath.c_str()); }
	static bool del(const std::wstring& fileOrFolder, std::wstring* pErr = nullptr);
	static bool create_dir(const std::wstring& thePath, std::wstring* pErr = nullptr);
	static bool unzip(const std::wstring& zipFile, const std::wstring& destFolder, std::wstring* pErr = nullptr);
	static std::vector<std::wstring> list_dir(const std::wstring& pathAndPattern);
	static std::vector<std::wstring> list_dir(const std::wstring& dirPath, const std::wstring& pattern);
};

}//namespace winutil