
#include "File.h"
#include <direct.h> // _wmkdir()
#include <Shlobj.h>
#pragma comment(lib, "Shell32.lib") // SHGetFolderPath

bool File::CreateDir(const wchar_t *path)
{
	return ::_wmkdir(path) == 0;
}

Date File::DateLastModified(const wchar_t *path)
{
	WIN32_FILE_ATTRIBUTE_DATA fad = { 0 };
	::GetFileAttributesEx(path, GetFileExInfoStandard, &fad);
	return Date(&fad.ftLastWriteTime); // already converted to current timezone
}

Date File::DateCreated(const wchar_t *path)
{
	WIN32_FILE_ATTRIBUTE_DATA fad = { 0 };
	::GetFileAttributesEx(path, GetFileExInfoStandard, &fad);
	return Date(&fad.ftCreationTime); // already converted to current timezone
}

String File::GetExePath()
{
	String ret;
	ret.reserve(MAX_PATH);

	::GetModuleFileName(0, ret.ptrAt(0), ret.reserved() + 1); // retrieves EXE itself directory
	ret[ ret.findr(L'\\') ] = L'\0'; // truncate removing EXE file name, remove trailing backslash

#ifdef _DEBUG
	ret[ ret.findr(L'\\') ] = L'\0'; // bypass "Debug" folder, remove trailing backslash
#endif

	return ret;
}

String File::GetDesktopPath()
{
	String ret;
	ret.reserve(MAX_PATH);
	::SHGetFolderPath(NULL, CSIDL_DESKTOPDIRECTORY, NULL, 0, ret.ptrAt(0)); // won't have trailing backslash
	return ret;
}

String File::GetMyDocsPath()
{
	String ret;
	ret.reserve(MAX_PATH);
	::SHGetFolderPath(NULL, CSIDL_MYDOCUMENTS, NULL, 0, ret.ptrAt(0)); // won't have trailing backslash
	return ret;
}

String File::GetRoamingPath()
{
	String ret;
	ret.reserve(MAX_PATH);
	::SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, ret.ptrAt(0)); // won't have trailing backslash
	return ret;
}

bool File::WriteUtf8(const wchar_t *path, const wchar_t *data, String *pErr)
{
	bool isUtf8 = false;
	int dataLen = ::lstrlen(data);
	for(int i = 0; i < dataLen; ++i) {
		if(data[i] > 127) {
			isUtf8 = true;
			break;
		}
	}
	
	File::Raw fout;
	if(!fout.open(path, Access::READWRITE, pErr))
		return false;
	if(fout.size() && !fout.setNewSize(0, pErr)) // if already exists, truncate to empty
		return false;
	
	// If the text doesn't have any char to make it UTF-8, it'll
	// be simply converted to plain ASCII.
	int newLen = ::WideCharToMultiByte(CP_UTF8, 0, data, dataLen, NULL, 0, NULL, NULL);
	Array<BYTE> outBuf(newLen + (isUtf8 ? 3 : 0));
	if(isUtf8)
		::memcpy(&outBuf[0], "\xEF\xBB\xBF", 3); // write UTF-8 BOM
	::WideCharToMultiByte(CP_UTF8, 0, data, dataLen, (char*)&outBuf[isUtf8 ? 3 : 0], newLen, NULL, NULL);
	if(!fout.write(&outBuf, pErr)) // one single write() to all data, better performance
		return false;
	
	if(pErr) *pErr = L"";
	return true;
}