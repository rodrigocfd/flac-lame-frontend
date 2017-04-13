/**
 * Part of WinLamb - Win32 API Lambda Library - More
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winlamb-more
 */

#pragma once
#include "file.h"
#include <Shlobj.h>

namespace wl {

// Utilities to work with zipped files.
class zip final {
protected:
	zip() = default;

public:
	static bool extract_all(const std::wstring& zipFile,
		const std::wstring& destFolder, std::wstring* pErr = nullptr)
	{
		if (!file::exists(zipFile)) {
			if (pErr) *pErr = str::format(L"File doesn't exist: \"%s\".", zipFile.c_str());
			return false;
		}
		if (!file::exists(destFolder)) {
			if (pErr) *pErr = str::format(L"Output directory doesn't exist: \"%s\".", destFolder.c_str());
			return false;
		}

		// http://social.msdn.microsoft.com/Forums/vstudio/en-US/45668d18-2840-4887-87e1-4085201f4103/visual-c-to-unzip-a-zip-file-to-a-specific-directory
		CoInitialize(nullptr);

		IShellDispatch* pISD = nullptr;
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

		Folder* pZippedFile = nullptr;
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

		Folder* pDestination = nullptr;
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

		FolderItems* pFilesInside = nullptr;
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

		IDispatch* pItem = nullptr;
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
};

}//namespace wl