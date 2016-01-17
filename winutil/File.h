
#pragma once
#include <vector>
#include <Windows.h>

class File final {
public:
	enum class Access { READONLY, READWRITE };
private:
	HANDLE _hFile;
	Access _access;
public:
	~File();
	File();
	File(File&& f);
	File&  operator=(File&& f);

	HANDLE hFile() const;
	Access getAccess() const;
	void   close();
	size_t size() const;
	bool   open(const wchar_t *path, Access access, std::wstring *pErr = nullptr);
	bool   open(const std::wstring& path, Access access, std::wstring *pErr = nullptr);
	bool   setNewSize(size_t newSize, std::wstring *pErr = nullptr);
	bool   truncate(std::wstring *pErr = nullptr);
	bool   getContent(std::vector<BYTE>& buf, std::wstring *pErr = nullptr) const;
	bool   write(const BYTE *pData, size_t sz, std::wstring *pErr = nullptr);
	bool   write(const std::vector<BYTE>& data, std::wstring *pErr = nullptr);
	bool   rewind(std::wstring *pErr = nullptr);

	static bool exists(const wchar_t *path);
	static bool exists(const std::wstring& path);
	static bool isDir(const wchar_t *path);
	static bool isDir(const std::wstring& path);
	static bool del(const wchar_t *path, std::wstring *pErr = nullptr);
	static bool del(const std::wstring& path, std::wstring *pErr = nullptr);
	static bool createDir(const wchar_t *path, std::wstring *pErr = nullptr);
	static bool createDir(const std::wstring& path, std::wstring *pErr = nullptr);
	static bool showOpen(HWND hWnd, const wchar_t *filter, std::wstring& buf);
	static bool showOpen(HWND hWnd, const wchar_t *filter, std::vector<std::wstring>& arrBuf);
	static bool showSave(HWND hWnd, const wchar_t *filter, std::wstring& buf, const wchar_t *defFile);
	static bool showChooseFolder(HWND hWnd, std::wstring& buf);
	static bool unzip(std::wstring zipFile, std::wstring destFolder, std::wstring *pErr = nullptr);
	static std::vector<std::wstring> listDir(std::wstring pathAndPattern);
	static std::vector<std::wstring> listDir(std::wstring path, std::wstring pattern);
};