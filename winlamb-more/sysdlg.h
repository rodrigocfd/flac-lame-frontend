/**
 * Part of WinLamb - Win32 API Lambda Library - More
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winlamb-more
 */

#pragma once
#include <algorithm>
#include "../winlamb/base_wnd.h"
#include "str.h"
#include <Shlobj.h>

namespace wl {

// Wrappers system dialogs calls.
class sysdlg final {
protected:
	sysdlg() = default;

public:
	static int msgbox(HWND hParent, std::wstring title,
		std::wstring text, UINT uType = 0)
	{
		if (hParent) { // the hook is set to center the message box window on parent
			_hWndParent.val = hParent;
			_hHookMsgBox.val = SetWindowsHookExW(WH_CBT, [](int code, WPARAM wp, LPARAM lp)->LRESULT {
				// http://www.codeguru.com/cpp/w-p/win32/messagebox/print.php/c4541
				if (code == HCBT_ACTIVATE) {
					HWND hMsgbox = reinterpret_cast<HWND>(wp);
					RECT rcMsgbox = { 0 }, rcParent = { 0 };

					if (hMsgbox && _hWndParent.val && GetWindowRect(hMsgbox, &rcMsgbox) && GetWindowRect(_hWndParent.val, &rcParent)) {
						RECT  rcScreen = { 0 };
						POINT pos = { 0 };
						SystemParametersInfoW(SPI_GETWORKAREA, 0, static_cast<PVOID>(&rcScreen), 0); // size of desktop

																									 // Adjusted x,y coordinates to message box window.
						pos.x = rcParent.left + (rcParent.right - rcParent.left) / 2 - (rcMsgbox.right - rcMsgbox.left) / 2;
						pos.y = rcParent.top + (rcParent.bottom - rcParent.top) / 2 - (rcMsgbox.bottom - rcMsgbox.top) / 2;

						// Screen out-of-bounds corrections.
						if (pos.x < 0) {
							pos.x = 0;
						} else if (pos.x + (rcMsgbox.right - rcMsgbox.left) > rcScreen.right) {
							pos.x = rcScreen.right - (rcMsgbox.right - rcMsgbox.left);
						}
						if (pos.y < 0) {
							pos.y = 0;
						} else if (pos.y + (rcMsgbox.bottom - rcMsgbox.top) > rcScreen.bottom) {
							pos.y = rcScreen.bottom - (rcMsgbox.bottom - rcMsgbox.top);
						}
						MoveWindow(hMsgbox, pos.x, pos.y,
							rcMsgbox.right - rcMsgbox.left, rcMsgbox.bottom - rcMsgbox.top,
							FALSE);
					}
					UnhookWindowsHookEx(_hHookMsgBox.val); // release hook
				}
				return CallNextHookEx(nullptr, code, wp, lp);
			}, nullptr, GetCurrentThreadId());
		}
		return MessageBoxW(hParent, text.c_str(), title.c_str(), uType);
	}

	static int msgbox(const base::wnd* parent, std::wstring title,
		std::wstring text, UINT uType = 0)
	{
		return msgbox(parent->hwnd(), title, text, uType);
	}

	static bool open_file(HWND hParent, const wchar_t* filterWithPipes,
		std::wstring& buf)
	{
		OPENFILENAME         ofn = { 0 };
		wchar_t              tmpBuf[MAX_PATH] = { L'\0' };
		std::vector<wchar_t> zfilter = _format_file_filter(filterWithPipes);

		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner   = hParent;
		ofn.lpstrFilter = &zfilter[0];
		ofn.lpstrFile   = tmpBuf;
		ofn.nMaxFile    = ARRAYSIZE(tmpBuf);
		ofn.Flags       = OFN_EXPLORER | OFN_ENABLESIZING | OFN_FILEMUSTEXIST;// | OFN_HIDEREADONLY;

		bool ret = GetOpenFileNameW(&ofn) != 0;
		if (ret) buf = tmpBuf;
		return ret;
	}

	static bool open_file(const base::wnd* parent, const wchar_t* filterWithPipes,
		std::wstring& buf)
	{
		return open_file(parent->hwnd(), filterWithPipes, buf);
	}

	static bool open_file(HWND hParent, const wchar_t* filterWithPipes,
		std::vector<std::wstring>& arrBuf)
	{
		OPENFILENAME         ofn = { 0 };
		std::vector<wchar_t> multiBuf(65536, L'\0'); // http://www.askjf.com/?q=2179s http://www.askjf.com/?q=2181s
		std::vector<wchar_t> zfilter = _format_file_filter(filterWithPipes);
		arrBuf.clear();

		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner   = hParent;
		ofn.lpstrFilter = &zfilter[0];
		ofn.lpstrFile   = &multiBuf[0];
		ofn.nMaxFile    = static_cast<DWORD>(multiBuf.size()); // including terminating null
		ofn.Flags       = OFN_FILEMUSTEXIST | OFN_ALLOWMULTISELECT | OFN_EXPLORER | OFN_ENABLESIZING;
		//ofn.FlagsEx = OFN_EX_NOPLACESBAR;
		// Call to GetOpenFileName() causes "First-chance exception (KernelBase.dll): The RPC server is unavailable."
		// in debug mode, but nothing else happens. The only way to get rid of it was using OFN_EX_NOPLACESBAR flag,
		// don't know why!

		if (GetOpenFileNameW(&ofn)) {
			std::vector<std::wstring> strs = str::explode_multi_zero(&multiBuf[0]);
			if (!strs.size()) {
				OutputDebugStringW(L"ERROR: GetOpenFileName didn't return multiple strings.\n");
				return false;
			}

			if (strs.size() == 1) { // if user selected only 1 file, the string is the full path, and that's all
				arrBuf.emplace_back(strs[0]);
			} else { // user selected 2 or more files
				std::wstring& basePath = strs[0]; // 1st string is the base path; others are the filenames
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
		OutputDebugStringW((errNo == FNERR_BUFFERTOOSMALL) ?
			L"ERROR: GetOpenFileName buffer is too small.\n" :
			str::format(L"ERROR: GetOpenFileName failed with error %u.\n", errNo).c_str() );
		return false;
	}

	static bool open_file(const base::wnd* parent, const wchar_t* filterWithPipes,
		std::vector<std::wstring>& arrBuf)
	{
		return open_file(parent->hwnd(), filterWithPipes, arrBuf);
	}
	
	static bool save_file(HWND hParent, const wchar_t* filterWithPipes,
		std::wstring& buf, const wchar_t* defFile)
	{
		OPENFILENAME         ofn = { 0 };
		wchar_t              tmpBuf[MAX_PATH] = { L'\0' };
		std::vector<wchar_t> zfilter = _format_file_filter(filterWithPipes);

		if (defFile) lstrcpyW(tmpBuf, defFile);

		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner   = hParent;
		ofn.lpstrFilter = &zfilter[0];
		ofn.lpstrFile   = tmpBuf;
		ofn.nMaxFile    = ARRAYSIZE(tmpBuf);
		ofn.Flags       = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
		ofn.lpstrDefExt = L"txt"; // apparently could be anything, will just force append of combo selected extension

		bool ret = GetSaveFileNameW(&ofn) != 0;
		if (ret) buf = tmpBuf;
		return ret;
	}

	static bool save_file(const base::wnd* parent, const wchar_t* filterWithPipes,
		std::wstring& buf, const wchar_t* defFile)
	{
		return save_file(parent->hwnd(), filterWithPipes, buf, defFile);
	}

	static bool choose_folder(HWND hParent, std::wstring& buf) {
		CoInitialize(nullptr);
		//LPITEMIDLIST pidlRoot = 0;
		//if (defFolder) SHParseDisplayName(defFolder, nullptr, &pidlRoot, 0, nullptr);

		BROWSEINFOW bi = { 0 };
		bi.hwndOwner = hParent;
		bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;

		PIDLIST_ABSOLUTE pidl = SHBrowseForFolderW(&bi);
		if (!pidl) return false; // user cancelled

		wchar_t tmpbuf[MAX_PATH] = { L'\0' };
		if (!SHGetPathFromIDListW(pidl, tmpbuf)) {
			return false; // some weird error
		}
		CoUninitialize();
		buf = tmpbuf;
		return true;
	}

	static bool choose_folder(const base::wnd* parent, std::wstring& buf) {
		return choose_folder(parent->hwnd(), buf);
	}

private:
	template<typename T>
	struct _static_holder final {
		static T val; // http://stackoverflow.com/a/11709860
	};

	static _static_holder<HHOOK> _hHookMsgBox;
	static _static_holder<HWND>  _hWndParent;

	static std::vector<wchar_t> _format_file_filter(const wchar_t* filterWithPipes) {
		// Input filter follows same C# syntax:
		// L"Text Files (*.txt)|*.txt|All Files (*.*)|*.*"
		std::vector<wchar_t> ret(lstrlenW(filterWithPipes) + 2, L'\0'); // two terminating nulls
		for (size_t i = 0; i < ret.size() - 1; ++i) {
			ret[i] = (filterWithPipes[i] != L'|') ? filterWithPipes[i] : L'\0';
		}
		return ret;
	}
};

template<typename T> T sysdlg::_static_holder<T>::val;

}//namespace wl