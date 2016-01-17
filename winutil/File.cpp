
#include <algorithm>
#include "File.h"
#include "Str.h"
#include <Shlobj.h>
using std::vector;
using std::wstring;

File::~File()
{
	close();
}

File::File()
	: _hFile(nullptr), _access(Access::READONLY)
{
}

File::File(File&& f)
	: _hFile(f._hFile), _access(f._access)
{
	f._hFile = nullptr;
	f._access = Access::READONLY;
}

File& File::operator=(File&& f)
{
	std::swap(_hFile, f._hFile);
	std::swap(_access, f._access);
	return *this;
}

HANDLE File::hFile() const
{
	return _hFile;
}

File::Access File::getAccess() const
{
	return _access;
}

void File::close()
{
	if (_hFile) {
		CloseHandle(_hFile);
		_hFile = nullptr;
		_access = Access::READONLY;
	}
}

size_t File::size() const
{
	return GetFileSize(_hFile, nullptr);
}

bool File::open(const wchar_t *path, Access access, wstring *pErr)
{
	close();
	_hFile = CreateFile(path,
		GENERIC_READ | (access == Access::READWRITE ? GENERIC_WRITE : 0),
		(access == Access::READWRITE) ? 0 : FILE_SHARE_READ, nullptr,
		(access == Access::READWRITE) ? OPEN_ALWAYS : OPEN_EXISTING,
		0, nullptr); // if file doesn't exist, will be created

	if (_hFile == INVALID_HANDLE_VALUE) {
		_hFile = nullptr;
		if (pErr) *pErr = Str::format(L"CreateFile() failed to open file as %s, error code %d.",
			(access == Access::READONLY) ? L"read-only" : L"read-write", GetLastError());
		return false;
	}

	_access = access; // keep for future checks
	if (pErr) pErr->clear();
	return true;
}

bool File::open(const wstring& path, Access access, wstring *pErr)
{
	return open(path.c_str(), access, pErr);
}

bool File::setNewSize(size_t newSize, wstring *pErr)
{
	// This method will truncate or expand the file, according to the new size.
	// Size zero will empty the file.

	if (!_hFile) {
		if (pErr) *pErr = L"File has not been opened.";
		return false;
	}

	if (_access == Access::READONLY) {
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
		if (pErr) *pErr = Str::format(L"SetFilePointer() failed with offset of %d, error code %d.", newSize, err);
		return false;
	}

	if (!SetEndOfFile(_hFile)) {
		DWORD err = GetLastError();
		close();
		if (pErr) *pErr = Str::format(L"SetEndOfFile() failed with offset of %d, error code %d.", newSize, err);
		return false;
	}

	r = SetFilePointer(_hFile, 0, nullptr, FILE_BEGIN); // rewind
	if (r == INVALID_SET_FILE_POINTER) {
		DWORD err = GetLastError();
		close();
		if (pErr) *pErr = Str::format(L"SetFilePointer() failed to rewind the file, error code %d.", err);
		return false;
	}

	if (pErr) pErr->clear();
	return true;
}

bool File::truncate(wstring *pErr)
{
	return setNewSize(0, pErr);
}

bool File::getContent(vector<BYTE>& buf, wstring *pErr) const
{
	if (!_hFile) {
		if (pErr) *pErr = L"File has not been opened.";
		return false;
	}

	buf.resize(size());
	DWORD bytesRead = 0;
	if (!ReadFile(_hFile, &buf[0], static_cast<DWORD>(buf.size()), &bytesRead, nullptr)) {
		if (pErr) *pErr = Str::format(L"ReadFile() failed to read %d bytes.", buf.size());
		return false;
	}

	if (pErr) pErr->clear();
	return true;
}

bool File::write(const BYTE *pData, size_t sz, wstring *pErr)
{
	if (!_hFile) {
		if (pErr) *pErr = L"File has not been opened.";
		return false;
	}

	if (_access == Access::READONLY) {
		if (pErr) *pErr = L"File is opened for read-only access.";
		return false;
	}

	// File boundary will be expanded if needed.
	// Internal file pointer will move forward.
	DWORD dwWritten = 0;
	if (!WriteFile(_hFile, pData, static_cast<DWORD>(sz), &dwWritten, nullptr)) {
		if (pErr) *pErr = Str::format(L"WriteFile() failed to write %d bytes.", sz);
		return false;
	}

	if (pErr) pErr->clear();
	return true;
}

bool File::write(const vector<BYTE>& data, wstring *pErr)
{
	return write(&data[0], data.size(), pErr);
}

bool File::rewind(wstring *pErr)
{
	if (!_hFile) {
		if (pErr) *pErr = L"File has not been opened.";
		return false;
	}

	if (SetFilePointer(_hFile, 0, nullptr, FILE_BEGIN) == INVALID_SET_FILE_POINTER) {
		if (pErr) *pErr = Str::format(L"SetFilePointer() failed, error code %d.", GetLastError());
		return false;
	}
	return true;
}

bool File::exists(const wchar_t *path)
{
	return GetFileAttributes(path) != INVALID_FILE_ATTRIBUTES;
}

bool File::exists(const wstring& path)
{
	return exists(path.c_str());
}

bool File::isDir(const wchar_t *path)
{
	return (GetFileAttributes(path) & FILE_ATTRIBUTE_DIRECTORY) != 0;
}

bool File::isDir(const wstring& path)
{
	return isDir(path.c_str());
}

bool File::del(const wchar_t *path, wstring *pErr)
{
	if (isDir(path)) {
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
			if (pErr) *pErr = Str::format(L"DeleteFile() failed, error code %d.", GetLastError());
			return false;
		}
	}
	if (pErr) pErr->clear();
	return true;
}

bool File::del(const wstring& path, wstring *pErr)
{
	return del(path.c_str(), pErr);
}

bool File::createDir(const wchar_t *path, wstring *pErr)
{
	if (!CreateDirectory(path, nullptr)) {
		if (pErr) *pErr = Str::format(L"CreateDirectory() failed, error code %d.", GetLastError());
		return false;
	}
	if (pErr) pErr->clear();
	return true;
}

bool File::createDir(const wstring& path, wstring *pErr)
{
	return createDir(path.c_str(), pErr);
}

static vector<wchar_t> _formatFileFilter(const wchar_t *filterWithPipes)
{
	// Input filter follows same C# syntax:
	// L"Text Files (*.txt)|*.txt|All Files (*.*)|*.*"

	vector<wchar_t> ret(lstrlen(filterWithPipes) + 2, L'\0'); // two terminating nulls
	for (size_t i = 0; i < ret.size() - 1; ++i) {
		ret[i] = (filterWithPipes[i] != L'|') ? filterWithPipes[i] : L'\0';
	}
	return ret;
}

bool File::showOpen(HWND hWnd, const wchar_t *filter, wstring& buf)
{
	OPENFILENAME    ofn = { 0 };
	wchar_t         tmpBuf[MAX_PATH] = { L'\0' };
	vector<wchar_t> zfilter = _formatFileFilter(filter);

	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner   = hWnd;
	ofn.lpstrFilter = &zfilter[0];
	ofn.lpstrFile   = tmpBuf;
	ofn.nMaxFile    = ARRAYSIZE(tmpBuf);
	ofn.Flags       = OFN_EXPLORER | OFN_ENABLESIZING | OFN_FILEMUSTEXIST;// | OFN_HIDEREADONLY;

	bool ret = GetOpenFileName(&ofn) != 0;
	if (ret) buf = tmpBuf;
	return ret;
}

bool File::showOpen(HWND hWnd, const wchar_t *filter, vector<wstring>& arrBuf)
{
	OPENFILENAME    ofn = { 0 };
	vector<wchar_t> multiBuf(65536, L'\0'); // http://www.askjf.com/?q=2179s http://www.askjf.com/?q=2181s
	vector<wchar_t> zfilter = _formatFileFilter(filter);
	arrBuf.clear();

	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner   = hWnd;
	ofn.lpstrFilter = &zfilter[0];
	ofn.lpstrFile   = &multiBuf[0];
	ofn.nMaxFile    = static_cast<DWORD>(multiBuf.size()); // including terminating null
	ofn.Flags       = OFN_FILEMUSTEXIST | OFN_ALLOWMULTISELECT | OFN_EXPLORER | OFN_ENABLESIZING;
	//ofn.FlagsEx = OFN_EX_NOPLACESBAR;
	// Call to GetOpenFileName() causes "First-chance exception (KernelBase.dll): The RPC server is unavailable."
	// in debug mode, but nothing else happens. The only way to get rid of it was using OFN_EX_NOPLACESBAR flag,
	// don't know why!

	if (GetOpenFileName(&ofn)) {
		auto explodeMultiStr = [](const wchar_t *multiStr)->vector<wstring> {
			// Example multiStr:
			// L"first one\0second one\0third one\0"
			// Assumes a well-formed multiStr, which ends with two nulls.

			// Count number of null-delimited strings; string end with double null.
			int numStrings = 0;
			const wchar_t *pRun = multiStr;
			while (*pRun) {
				++numStrings;
				pRun += lstrlen(pRun) + 1;
			}

			// Alloc return array of strings.
			vector<wstring> ret;
			ret.reserve(numStrings);

			// Copy each string.
			pRun = multiStr;
			for (int i = 0; i < numStrings; ++i) {
				ret.emplace_back(pRun);
				pRun += lstrlen(pRun) + 1;
			}

			return ret;
		};

		vector<wstring> strs = explodeMultiStr(&multiBuf[0]);
		if (!strs.size()) {
			MessageBox(hWnd, L"GetOpenFileName didn't return multiple strings.", L"Error", MB_ICONERROR);
			return false;
		}

		if (strs.size() == 1) { // if user selected only 1 file, the string is the full path, and that's all
			arrBuf.emplace_back(strs[0]);
		} else { // user selected 2 or more files
			wstring& basePath = strs[0]; // 1st string is the base path; others are the filenames
			arrBuf.resize(strs.size() - 1); // alloc return buffer

			for (size_t i = 0; i < strs.size() - 1; ++i) {
				arrBuf[i].reserve(basePath.size() + strs[i + 1].size() + 1); // room for backslash
				arrBuf[i] = basePath;
				arrBuf[i].append(L"\\").append(strs[i + 1]); // concat folder + file
			}
			std::sort(arrBuf.begin(), arrBuf.end());
		}
		return true; // all good
	}

	DWORD errNo = CommDlgExtendedError();
	if (errNo == FNERR_BUFFERTOOSMALL) {
		MessageBox(hWnd, L"GetOpenFileName: buffer too small.", L"Error", MB_ICONERROR);
	} else if (errNo) {
		wchar_t txtBuf[64] = { L'\0' };
		wsprintf(txtBuf, L"GetOpenFileName: failed with error %d.", errNo);
		MessageBox(hWnd, txtBuf, L"Error", MB_ICONERROR);
	}
	return false;
}

bool File::showSave(HWND hWnd, const wchar_t *filter, wstring& buf, const wchar_t *defFile)
{
	OPENFILENAME    ofn = { 0 };
	wchar_t         tmpBuf[MAX_PATH] = { L'\0' };
	vector<wchar_t> zfilter = _formatFileFilter(filter);

	if (defFile) lstrcpy(tmpBuf, defFile);

	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner   = hWnd;
	ofn.lpstrFilter = &zfilter[0];
	ofn.lpstrFile   = tmpBuf;
	ofn.nMaxFile    = ARRAYSIZE(tmpBuf);
	ofn.Flags       = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
	ofn.lpstrDefExt = L"txt"; // apparently could be anything, will just force append of combo selected extension

	bool ret = GetSaveFileName(&ofn) != 0;
	if (ret) buf = tmpBuf;
	return ret;
}

bool File::showChooseFolder(HWND hWnd, wstring& buf)
{
	CoInitialize(nullptr);

	//LPITEMIDLIST pidlRoot = 0;
	//if (defFolder) SHParseDisplayName(defFolder, nullptr, &pidlRoot, 0, nullptr);

	BROWSEINFO bi = { 0 };
	bi.hwndOwner = hWnd;
	bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;

	PIDLIST_ABSOLUTE pidl = SHBrowseForFolder(&bi);
	if (!pidl) return false; // user cancelled

	wchar_t tmpbuf[MAX_PATH] = { L'\0' };
	if (!SHGetPathFromIDList(pidl, tmpbuf)) {
		return false; // some weird error
	}

	CoUninitialize();
	buf = tmpbuf;
	return true;
}

bool File::unzip(wstring zipFile, wstring destFolder, wstring *pErr)
{
	if (!exists(zipFile)) {
		if (pErr) *pErr = Str::format(L"File doesn't exist: \"%s\".", zipFile.c_str());
		return false;
	}
	if (!exists(destFolder)) {
		if (pErr) *pErr = Str::format(L"Output directory doesn't exist: \"%s\".", destFolder.c_str());
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

vector<wstring> File::listDir(wstring pathAndPattern)
{
	// Entry example: "C:\\myfolder\\*.mp3"
	
	vector<wstring> files;

	WIN32_FIND_DATA wfd = { 0 };
	HANDLE hFind = FindFirstFile(pathAndPattern.c_str(), &wfd);
	if (!hFind) return files; // nothing found

	wstring path = pathAndPattern;
	path = Str::folderFromPath(path);

	do {
		if (*wfd.cFileName) {
			files.emplace_back(path);
			files.back().append(L"\\").append(wfd.cFileName);
		}
	} while (FindNextFile(hFind, &wfd));

	FindClose(hFind);
	return files;
}

vector<wstring> File::listDir(wstring path, wstring pattern)
{
	if (path.back() != L'\\') path.append(L"\\");
	path.append(pattern);
	return listDir(path);
}