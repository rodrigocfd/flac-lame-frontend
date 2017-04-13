/**
 * Part of WinLamb - Win32 API Lambda Library - More
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winlamb-more
 */

#pragma once
#include "datetime.h"
#include "path.h"

namespace wl {

// Wrapper to a HANDLE of a file.
class file final {
public:
	enum class access { READONLY, READWRITE };
private:
	HANDLE _hFile;
	access _access;
	size_t _sz;

public:
	~file() { this->close(); }
	file() : _hFile(nullptr), _access(access::READONLY), _sz(-1) { }

	file(file&& f) : _hFile(f._hFile), _access(f._access), _sz(f._sz) {
		f._hFile = nullptr;
		f._access = access::READONLY;
		f._sz = -1;
	}

	file& operator=(file&& f) {
		std::swap(this->_hFile, f._hFile);
		std::swap(this->_access, f._access);
		std::swap(this->_sz, f._sz);
		return *this;
	}

	HANDLE hfile() const       { return this->_hFile; }
	access access_type() const { return this->_access; }

	void close() {
		if (this->_hFile) {
			CloseHandle(this->_hFile);
			this->_hFile = nullptr;
			this->_access = access::READONLY;
			this->_sz = -1; // http://stackoverflow.com/a/19483690
		}
	}

	size_t size() {
		if (this->_sz == -1) {
			this->_sz = GetFileSize(this->_hFile, nullptr); // cache
		}
		return this->_sz;
	}

	bool open_existing(const std::wstring& filePath, access accessType, std::wstring* pErr = nullptr) {
		if (!file::exists(filePath)) {
			if (pErr) *pErr = L"File doesn't exist.";
			return false;
		}

		if (!_raw_open(filePath,
			GENERIC_READ | (accessType == access::READWRITE ? GENERIC_WRITE : 0),
			(accessType == access::READWRITE) ? 0 : FILE_SHARE_READ,
			OPEN_EXISTING, pErr) ) // fails if file doesn't exist
		{
			return false;
		}

		this->_access = accessType; // keep for future checks
		if (pErr) pErr->clear();
		return true;
	}

	bool open_or_create(const std::wstring& filePath, std::wstring* pErr = nullptr) {
		// Intended to be called when you want to save a file, so it's always READWRITE.
		if (!_raw_open(filePath, GENERIC_READ | GENERIC_WRITE, 0, OPEN_ALWAYS, pErr)) {
			return false;
		}
		this->_access = access::READWRITE; // keep for future checks
		if (pErr) pErr->clear();
		return true;
	}

	bool set_new_size(size_t newSize, std::wstring* pErr = nullptr) {
		// This method will truncate or expand the file, according to the new size.
		// Size zero will empty the file.
		if (!this->_hFile) {
			if (pErr) *pErr = L"File has not been opened.";
			return false;
		}

		if (this->_access == access::READONLY) {
			if (pErr) *pErr = L"File is opened for read-only access.";
			return false;
		}

		if (this->size() == newSize) {
			return true; // nothing to do
		}

		DWORD r = SetFilePointer(this->_hFile, static_cast<LONG>(newSize), nullptr, FILE_BEGIN);
		if (r == INVALID_SET_FILE_POINTER) {
			DWORD err = GetLastError();
			this->close();
			if (pErr) *pErr = str::format(L"SetFilePointer() failed with offset of %u, error code %u.", newSize, err);
			return false;
		}

		if (!SetEndOfFile(this->_hFile)) {
			DWORD err = GetLastError();
			this->close();
			if (pErr) *pErr = str::format(L"SetEndOfFile() failed with offset of %u, error code %u.", newSize, err);
			return false;
		}

		r = SetFilePointer(this->_hFile, 0, nullptr, FILE_BEGIN); // rewind
		if (r == INVALID_SET_FILE_POINTER) {
			DWORD err = GetLastError();
			this->close();
			if (pErr) *pErr = str::format(L"SetFilePointer() failed to rewind the file, error code %u.", err);
			return false;
		}

		this->_sz = newSize; // update
		if (pErr) pErr->clear();
		return true;
	}

	bool rewind(std::wstring* pErr = nullptr) {
		if (!this->_hFile) {
			if (pErr) *pErr = L"File has not been opened.";
			return false;
		}

		if (SetFilePointer(this->_hFile, 0, nullptr, FILE_BEGIN) == INVALID_SET_FILE_POINTER) {
			if (pErr) *pErr = str::format(L"SetFilePointer() failed, error code %u.", GetLastError());
			return false;
		}
		return true;
	}

	bool read(std::vector<BYTE>& buf, std::wstring* pErr = nullptr) {
		if (!this->_hFile) {
			if (pErr) *pErr = L"File has not been opened.";
			return false;
		}

		buf.resize(this->size());
		DWORD bytesRead = 0;
		if (!ReadFile(this->_hFile, &buf[0], static_cast<DWORD>(buf.size()), &bytesRead, nullptr)) {
			if (pErr) *pErr = str::format(L"ReadFile() failed to read %u bytes.", buf.size());
			return false;
		}

		if (pErr) pErr->clear();
		return true;
	}

	bool write(const BYTE* pData, size_t sz, std::wstring* pErr = nullptr) {
		if (!this->_hFile) {
			if (pErr) *pErr = L"File has not been opened.";
			return false;
		}

		if (this->_access == access::READONLY) {
			if (pErr) *pErr = L"File is opened for read-only access.";
			return false;
		}

		// File boundary will be expanded if needed.
		// Internal file pointer will move forward.
		DWORD dwWritten = 0;
		if (!WriteFile(this->_hFile, pData, static_cast<DWORD>(sz), &dwWritten, nullptr)) {
			if (pErr) *pErr = str::format(L"WriteFile() failed to write %u bytes.", sz);
			return false;
		}

		this->_sz = -1; // reset
		if (pErr) pErr->clear();
		return true;
	}

	bool write(const std::vector<BYTE>& data, std::wstring* pErr = nullptr) {
		return this->write(&data[0], data.size(), pErr);
	}

	bool get_times(datetime* creation, datetime* lastAccess, datetime* lastWrite) const {
		if (!this->_hFile) return false;

		FILETIME ftCreation = { 0 },
			ftLastAccess = { 0 },
			ftLastWrite = { 0 };
		if (!GetFileTime(this->_hFile,
			creation ? &ftCreation : nullptr,
			lastAccess ? &ftLastAccess : nullptr,
			lastWrite ? &ftLastWrite : nullptr)) return false;

		if (creation) *creation = ftCreation;
		if (lastAccess) *lastAccess = ftLastAccess;
		if (lastWrite) *lastWrite = ftLastWrite;
		return true;
	}

private:
	bool _raw_open(const std::wstring& filePath, DWORD desiredAccess, DWORD shareMode,
		DWORD creationDisposition, std::wstring* pErr = nullptr)
	{
		if (filePath.empty()) {
			if (pErr) *pErr = L"Path is empty.";
			return false;
		}

		this->close();
		this->_hFile = CreateFileW(filePath.c_str(), desiredAccess, shareMode,
			nullptr, creationDisposition, 0, nullptr);

		if (this->_hFile == INVALID_HANDLE_VALUE) {
			this->_hFile = nullptr;
			if (pErr) *pErr = str::format(L"CreateFile() failed to open file as %s, error code %u.",
				(desiredAccess & GENERIC_WRITE) ? L"read-only" : L"read-write",
				GetLastError());
			return false;
		}

		if (pErr) pErr->clear();
		return true;
	}

public:
	static bool quick_read(const std::wstring& srcFilePath, std::vector<BYTE>& buf, std::wstring* pErr = nullptr) {
		file fin;
		if (!fin.open_existing(srcFilePath, access::READONLY, pErr)) return false;
		if (!fin.read(buf, pErr)) return false;
		fin.close();
		return true;
	}

	static bool quick_read(const std::wstring& srcFilePath, std::wstring& buf, std::wstring* pErr = nullptr) {
		std::vector<BYTE> blob;
		if (!quick_read(srcFilePath, blob, pErr)) return false;
		std::wstring ret;
		if (!str::parse_blob(blob, ret, pErr)) return false;
		return true;
	}

	static bool quick_write(const std::wstring& destFilePath, const BYTE* pData, size_t sz, std::wstring* pErr = nullptr) {
		file fout;
		if (!fout.open_or_create(destFilePath, pErr)) return false;
		if (!fout.set_new_size(sz, pErr)) return false;
		if (!fout.write(pData, sz, pErr)) return false;
		fout.close();
		return true;
	}

	static bool quick_write(const std::wstring& destFilePath, const std::vector<BYTE>& data, std::wstring* pErr = nullptr) {
		return quick_write(destFilePath, &data[0], data.size(), pErr);
	}

	static bool quick_write_utf8(const std::wstring& destFilePath, const std::wstring& data, bool writeBom, std::wstring* pErr = nullptr) {
		std::vector<BYTE> blob = str::to_utf8_blob(data, writeBom);
		return quick_write(destFilePath, blob, pErr);
	}

	static bool exists(const wchar_t* fileOrFolder) {
		return GetFileAttributesW(fileOrFolder) != INVALID_FILE_ATTRIBUTES;
	}

	static bool exists(const std::wstring& fileOrFolder) {
		return exists(fileOrFolder.c_str());
	}

	static bool is_dir(const wchar_t* thePath) {
		return (GetFileAttributesW(thePath) & FILE_ATTRIBUTE_DIRECTORY) != 0;
	}
	
	static bool is_dir(const std::wstring& thePath) {
		return is_dir(thePath.c_str());
	}
	
	static bool is_hidden(const wchar_t* thePath) {
		return (GetFileAttributesW(thePath) & FILE_ATTRIBUTE_HIDDEN) != 0;
	}

	static bool is_hidden(const std::wstring& thePath) {
		return is_hidden(thePath.c_str());
	}

	static bool del(const std::wstring& fileOrFolder, std::wstring* pErr = nullptr) {
		if (is_dir(fileOrFolder)) {
			// http://stackoverflow.com/q/1468774/6923555
			wchar_t szDir[MAX_PATH + 1] = { L'\0' }; // +1 for the double null terminate
			lstrcpyW(szDir, fileOrFolder.c_str());

			SHFILEOPSTRUCTW fos = { 0 };
			fos.wFunc  = FO_DELETE;
			fos.pFrom  = szDir;
			fos.fFlags = FOF_NO_UI;

			if (!SHFileOperationW(&fos)) {
				if (pErr) *pErr = L"SHFileOperation() failed to recursively delete directory.";
				return false;
			}
		} else {
			if (!DeleteFileW(fileOrFolder.c_str())) {
				if (pErr) *pErr = str::format(L"DeleteFile() failed, error code %u.", GetLastError());
				return false;
			}
		}
		if (pErr) pErr->clear();
		return true;
	}

	static bool create_dir(const std::wstring& thePath, std::wstring* pErr = nullptr) {
		if (!CreateDirectoryW(thePath.c_str(), nullptr)) {
			if (pErr) *pErr = str::format(L"CreateDirectory() failed, error code %u.", GetLastError());
			return false;
		}
		if (pErr) pErr->clear();
		return true;
	}

	static std::vector<std::wstring> list_dir(const std::wstring& pathAndPattern) {
		// Entry example: "C:\\myfolder\\*.mp3"
		std::vector<std::wstring> files;

		WIN32_FIND_DATA wfd = { 0 };
		HANDLE hFind = FindFirstFileW(pathAndPattern.c_str(), &wfd);
		if (!hFind) return files; // nothing found

		std::wstring pathPat = pathAndPattern;
		pathPat = path::folder_from(pathPat);

		do {
			if (*wfd.cFileName) {
				files.emplace_back(pathPat);
				files.back().append(L"\\").append(wfd.cFileName);
			}
		} while (FindNextFileW(hFind, &wfd));

		FindClose(hFind);
		return files;
	}

	static std::vector<std::wstring> list_dir(const std::wstring& dirPath, const std::wstring& pattern) {
		std::wstring pathAndPattern = dirPath;
		if (pathAndPattern.back() != L'\\') pathAndPattern.append(L"\\");
		pathAndPattern.append(pattern);
		return list_dir(pathAndPattern);
	}
};

}//namespace wl