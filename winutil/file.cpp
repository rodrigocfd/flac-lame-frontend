/**
 * Part of WinUtil - Windows Utilities
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winutil
 */

#include <algorithm>
#include "file.h"
#include "path.h"
#include "str.h"
#include <Shlobj.h>
using namespace winutil;
using std::vector;
using std::wstring;

file::file()
	: _hFile(nullptr), _access(access::READONLY)
{
}

file::file(file&& f)
	: _hFile(f._hFile), _access(f._access)
{
	f._hFile = nullptr;
	f._access = access::READONLY;
}

file& file::operator=(file&& f)
{
	std::swap(_hFile, f._hFile);
	std::swap(_access, f._access);
	return *this;
}

void file::close()
{
	if (_hFile) {
		CloseHandle(_hFile);
		_hFile = nullptr;
		_access = access::READONLY;
	}
}

bool file::open(const wchar_t *path, access access, wstring *pErr)
{
	close();
	_hFile = CreateFile(path,
		GENERIC_READ | (access == access::READWRITE ? GENERIC_WRITE : 0),
		(access == access::READWRITE) ? 0 : FILE_SHARE_READ, nullptr,
		(access == access::READWRITE) ? OPEN_ALWAYS : OPEN_EXISTING,
		0, nullptr); // if file doesn't exist, will be created

	if (_hFile == INVALID_HANDLE_VALUE) {
		_hFile = nullptr;
		if (pErr) *pErr = str::format(L"CreateFile() failed to open file as %s, error code %d.",
			(access == access::READONLY) ? L"read-only" : L"read-write", GetLastError());
		return false;
	}

	_access = access; // keep for future checks
	if (pErr) pErr->clear();
	return true;
}

bool file::set_new_size(size_t newSize, wstring *pErr)
{
	// This method will truncate or expand the file, according to the new size.
	// Size zero will empty the file.

	if (!_hFile) {
		if (pErr) *pErr = L"File has not been opened.";
		return false;
	}

	if (_access == access::READONLY) {
		if (pErr) *pErr = L"File is opened for read-only access.";
		return false;
	}

	if (size() == newSize) {
		return true; // nothing to do
	}

	DWORD r = SetFilePointer(_hFile, static_cast<LONG>(newSize), nullptr, FILE_BEGIN);
	if (r == INVALID_SET_FILE_POINTER) {
		DWORD err = GetLastError();
		close();
		if (pErr) *pErr = str::format(L"SetFilePointer() failed with offset of %d, error code %d.", newSize, err);
		return false;
	}

	if (!SetEndOfFile(_hFile)) {
		DWORD err = GetLastError();
		close();
		if (pErr) *pErr = str::format(L"SetEndOfFile() failed with offset of %d, error code %d.", newSize, err);
		return false;
	}

	r = SetFilePointer(_hFile, 0, nullptr, FILE_BEGIN); // rewind
	if (r == INVALID_SET_FILE_POINTER) {
		DWORD err = GetLastError();
		close();
		if (pErr) *pErr = str::format(L"SetFilePointer() failed to rewind the file, error code %d.", err);
		return false;
	}

	if (pErr) pErr->clear();
	return true;
}

bool file::get_content(vector<BYTE>& buf, wstring *pErr) const
{
	if (!_hFile) {
		if (pErr) *pErr = L"File has not been opened.";
		return false;
	}

	buf.resize(size());
	DWORD bytesRead = 0;
	if (!ReadFile(_hFile, &buf[0], static_cast<DWORD>(buf.size()), &bytesRead, nullptr)) {
		if (pErr) *pErr = str::format(L"ReadFile() failed to read %d bytes.", buf.size());
		return false;
	}

	if (pErr) pErr->clear();
	return true;
}

bool file::write(const BYTE *pData, size_t sz, wstring *pErr)
{
	if (!_hFile) {
		if (pErr) *pErr = L"File has not been opened.";
		return false;
	}

	if (_access == access::READONLY) {
		if (pErr) *pErr = L"File is opened for read-only access.";
		return false;
	}

	// File boundary will be expanded if needed.
	// Internal file pointer will move forward.
	DWORD dwWritten = 0;
	if (!WriteFile(_hFile, pData, static_cast<DWORD>(sz), &dwWritten, nullptr)) {
		if (pErr) *pErr = str::format(L"WriteFile() failed to write %d bytes.", sz);
		return false;
	}

	if (pErr) pErr->clear();
	return true;
}

bool file::rewind(wstring *pErr)
{
	if (!_hFile) {
		if (pErr) *pErr = L"File has not been opened.";
		return false;
	}

	if (SetFilePointer(_hFile, 0, nullptr, FILE_BEGIN) == INVALID_SET_FILE_POINTER) {
		if (pErr) *pErr = str::format(L"SetFilePointer() failed, error code %d.", GetLastError());
		return false;
	}
	return true;
}

bool file::del(const wchar_t *path, wstring *pErr)
{
	if (is_dir(path)) {
		// http://stackoverflow.com/questions/1468774/why-am-i-having-problems-recursively-deleting-directories
		wchar_t szDir[MAX_PATH + 1] = { L'\0' }; // +1 for the double null terminate
		lstrcpy(szDir, path);

		SHFILEOPSTRUCTW fos = { 0 };
		fos.wFunc  = FO_DELETE;
		fos.pFrom  = szDir;
		fos.fFlags = FOF_NO_UI;

		if (!SHFileOperation(&fos)) {
			if (pErr) *pErr = L"SHFileOperation() failed to recursively delete directory.";
			return false;
		}
	} else {
		if (!DeleteFile(path)) {
			if (pErr) *pErr = str::format(L"DeleteFile() failed, error code %d.", GetLastError());
			return false;
		}
	}
	if (pErr) pErr->clear();
	return true;
}

bool file::create_dir(const wchar_t *path, wstring *pErr)
{
	if (!CreateDirectory(path, nullptr)) {
		if (pErr) *pErr = str::format(L"CreateDirectory() failed, error code %d.", GetLastError());
		return false;
	}
	if (pErr) pErr->clear();
	return true;
}

bool file::unzip(wstring zipFile, wstring destFolder, wstring *pErr)
{
	if (!exists(zipFile)) {
		if (pErr) *pErr = str::format(L"File doesn't exist: \"%s\".", zipFile.c_str());
		return false;
	}
	if (!exists(destFolder)) {
		if (pErr) *pErr = str::format(L"Output directory doesn't exist: \"%s\".", destFolder.c_str());
		return false;
	}

	// http://social.msdn.microsoft.com/Forums/vstudio/en-US/45668d18-2840-4887-87e1-4085201f4103/visual-c-to-unzip-a-zip-file-to-a-specific-directory
	CoInitialize(nullptr);

	IShellDispatch *pISD = nullptr;
	if (FAILED(CoCreateInstance(CLSID_Shell, nullptr, CLSCTX_INPROC_SERVER,
		IID_IShellDispatch, reinterpret_cast<void**>(&pISD))))
	{
		if (pErr) *pErr = L"CoCreateInstance failed on IID_IShellDispatch.";
		return false;
	}

	BSTR bstrZipFile = SysAllocString(zipFile.c_str());
	VARIANT inZipFile = { 0 };
	inZipFile.vt = VT_BSTR;
	inZipFile.bstrVal = bstrZipFile;

	Folder *pZippedFile = nullptr;
	pISD->NameSpace(inZipFile, &pZippedFile);
	if (!pZippedFile) {
		SysFreeString(bstrZipFile);
		pISD->Release();
		CoUninitialize();
		if (pErr) *pErr = L"IShellDispatch::NameSpace() failed on zip file name.";
		return false;
	}

	BSTR bstrFolder = SysAllocString(destFolder.c_str());
	VARIANT outFolder = { 0 };
	outFolder.vt = VT_BSTR;
	outFolder.bstrVal = bstrFolder;

	Folder *pDestination = nullptr;
	pISD->NameSpace(outFolder, &pDestination);
	if (!pDestination) {
		SysFreeString(bstrFolder);
		pZippedFile->Release();
		SysFreeString(bstrZipFile);
		pISD->Release();
		CoUninitialize();
		if (pErr) *pErr = L"IShellDispatch::NameSpace() failed on directory name.";
		return false;
	}

	FolderItems *pFilesInside = nullptr;
	pZippedFile->Items(&pFilesInside);
	if (!pFilesInside) {
		pDestination->Release();
		SysFreeString(bstrFolder);
		pZippedFile->Release();
		SysFreeString(bstrZipFile);
		pISD->Release();
		CoUninitialize();
		if (pErr) *pErr = L"Folder::Items() failed.";
		return false;
	}

	long FilesCount = 0;
	pFilesInside->get_Count(&FilesCount);
	if (FilesCount < 1) {
		pFilesInside->Release();
		pDestination->Release();
		SysFreeString(bstrFolder);
		pZippedFile->Release();
		SysFreeString(bstrZipFile);
		pISD->Release();
		CoUninitialize();
		if (pErr) *pErr = L"FolderItems::get_Count() failed.";
		return false;
	}

	IDispatch *pItem = nullptr;
	pFilesInside->QueryInterface(IID_IDispatch, reinterpret_cast<void**>(&pItem));

	VARIANT item = { 0 };
	item.vt = VT_DISPATCH;
	item.pdispVal = pItem;

	VARIANT options = { 0 };
	options.vt = VT_I4;
	options.lVal = 1024 | 512 | 16 | 4; // http://msdn.microsoft.com/en-us/library/bb787866(VS.85).aspx

	bool okay = SUCCEEDED(pDestination->CopyHere(item, options));

	pItem->Release();
	pFilesInside->Release();
	pDestination->Release();
	SysFreeString(bstrFolder);
	pZippedFile->Release();
	SysFreeString(bstrZipFile);
	pISD->Release();
	CoUninitialize();

	if (!okay) {
		if (pErr) *pErr = L"Folder::CopyHere() failed.";
		return false;
	}
	if (pErr) pErr->clear();
	return true;
}

vector<wstring> file::list_dir(wstring pathAndPattern)
{
	// Entry example: "C:\\myfolder\\*.mp3"
	
	vector<wstring> files;

	WIN32_FIND_DATA wfd = { 0 };
	HANDLE hFind = FindFirstFile(pathAndPattern.c_str(), &wfd);
	if (!hFind) return files; // nothing found

	wstring pathPat = pathAndPattern;
	pathPat = path::folder_from(pathPat);

	do {
		if (*wfd.cFileName) {
			files.emplace_back(pathPat);
			files.back().append(L"\\").append(wfd.cFileName);
		}
	} while (FindNextFile(hFind, &wfd));

	FindClose(hFind);
	return files;
}

vector<wstring> file::list_dir(wstring path, wstring pattern)
{
	if (path.back() != L'\\') path.append(L"\\");
	path.append(pattern);
	return list_dir(path);
}