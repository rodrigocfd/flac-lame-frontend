/**
 * Part of WinUtil - Windows Utilities
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winutil
 */

#include <algorithm>
#include "sys.h"
#include "str.h"
#include <process.h>
#include <Shlobj.h>
using namespace winutil;
using std::initializer_list;
using std::function;
using std::vector;
using std::wstring;

void sys::thread(function<void()> callback)
{
	// Cheap alternative to std::thread([](){}).detach().

	struct CbPack { function<void()> cb; };
	CbPack* pack = new CbPack{ std::move(callback) };

	HANDLE thandle = reinterpret_cast<HANDLE>(_beginthreadex(nullptr, 0, [](void* ptr)->unsigned int {
		CbPack* pPack = reinterpret_cast<CbPack*>(ptr);
		pPack->cb(); // invoke user callback
		delete pPack;
		_endthreadex(0); // http://www.codeproject.com/Articles/7732/A-class-to-synchronise-thread-completions/
		return 0;
	}, pack, 0, nullptr));

	CloseHandle(thandle);
}

DWORD sys::exec(wstring cmdLine)
{
	SECURITY_ATTRIBUTES sa = { 0 };
	sa.nLength = sizeof(sa);
	sa.bInheritHandle = TRUE;

	STARTUPINFO si = { 0 };
	si.cb = sizeof(si);
	si.dwFlags = STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_SHOW;

	PROCESS_INFORMATION pi = { 0 };
	DWORD dwExitCode = 1; // returned by executed program

	// http://blogs.msdn.com/b/oldnewthing/archive/2009/06/01/9673254.aspx
	if (CreateProcess(nullptr, &cmdLine[0], &sa, nullptr, FALSE, 0, nullptr, nullptr, &si, &pi)) {
		WaitForSingleObject(pi.hProcess, INFINITE); // the program flow is stopped here to wait
		GetExitCodeProcess(pi.hProcess, &dwExitCode);
		CloseHandle(pi.hThread);
		CloseHandle(pi.hProcess);
	}
	return dwExitCode;
}

DWORD sys::exec_shell(wstring file)
{
	return static_cast<DWORD>(reinterpret_cast<INT_PTR>(
		ShellExecute(nullptr, L"open", file.c_str(), nullptr, nullptr, SW_SHOWNORMAL) ));
}

vector<wstring> sys::get_cmd_line()
{
	return str::explode_quoted(GetCommandLine());
}

wstring sys::path_of_exe()
{
	wchar_t buf[MAX_PATH] = { L'\0' };
	GetModuleFileName(nullptr, buf, ARRAYSIZE(buf)); // retrieves EXE itself directory

	wstring ret = buf;
	ret.resize(ret.find_last_of(L'\\')); // truncate removing EXE filename and trailing backslash
#ifdef _DEBUG
	ret.resize(ret.find_last_of(L'\\')); // bypass "Debug" folder, remove trailing backslash too
#endif
	return ret;
}

wstring sys::path_of_desktop()
{
	wchar_t buf[MAX_PATH] = { L'\0' };
	SHGetFolderPath(nullptr, CSIDL_DESKTOPDIRECTORY, nullptr, 0, buf); // won't have trailing backslash
	return buf;
}

static HHOOK _hHookMsgBox = nullptr;
static HWND  _hWndParent = nullptr;
static LRESULT CALLBACK _msgBoxHookProc(int code, WPARAM wp, LPARAM lp)
{
	// http://www.codeguru.com/cpp/w-p/win32/messagebox/print.php/c4541
	if (code == HCBT_ACTIVATE) {
		HWND hMsgbox = reinterpret_cast<HWND>(wp);
		RECT rcMsgbox = { 0 }, rcParent = { 0 };

		if (hMsgbox && _hWndParent && GetWindowRect(hMsgbox, &rcMsgbox) && GetWindowRect(_hWndParent, &rcParent)) {
			RECT  rcScreen = { 0 };
			POINT pos = { 0 };
			SystemParametersInfo(SPI_GETWORKAREA, 0, static_cast<PVOID>(&rcScreen), 0); // size of desktop

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
		UnhookWindowsHookEx(_hHookMsgBox); // release hook
	}
	return CallNextHookEx(nullptr, code, wp, lp);
}

int sys::msg_box(HWND hParent, wstring title, wstring text, UINT uType)
{
	if (hParent) { // the hook is set to center the message box window on parent
		_hWndParent = hParent;
		_hHookMsgBox = SetWindowsHookEx(WH_CBT, _msgBoxHookProc, nullptr, GetCurrentThreadId());
	}
	return MessageBox(hParent, text.c_str(), title.c_str(), uType);
}

static vector<wchar_t> _formatFileFilter(const wchar_t* filterWithPipes)
{
	// Input filter follows same C# syntax:
	// L"Text Files (*.txt)|*.txt|All Files (*.*)|*.*"

	vector<wchar_t> ret(lstrlen(filterWithPipes) + 2, L'\0'); // two terminating nulls
	for (size_t i = 0; i < ret.size() - 1; ++i) {
		ret[i] = (filterWithPipes[i] != L'|') ? filterWithPipes[i] : L'\0';
	}
	return ret;
}

bool sys::show_open_file(HWND hWnd, const wchar_t* filter, wstring& buf)
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

bool sys::show_open_file(HWND hWnd, const wchar_t* filter, vector<wstring>& arrBuf)
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
		vector<wstring> strs = str::explode_multi_zero(&multiBuf[0]);
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
		wsprintf(txtBuf, L"GetOpenFileName: failed with error %u.", errNo);
		MessageBox(hWnd, txtBuf, L"Error", MB_ICONERROR);
	}
	return false;
}

bool sys::show_save_file(HWND hWnd, const wchar_t* filter, wstring& buf, const wchar_t* defFile)
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

bool sys::show_choose_folder(HWND hWnd, wstring& buf)
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

static LRESULT CALLBACK _wheelHoverProc(HWND hChild, UINT msg, WPARAM wp, LPARAM lp, UINT_PTR idSubclass, DWORD_PTR refData)
{
	switch (msg)
	{
	case WM_MOUSEWHEEL:
		if (!(LOWORD(wp) & 0x0800)) { // bitflag not set, this is the first and unprocessed WM_MOUSEWHEEL passage
			HWND hParent = reinterpret_cast<HWND>(refData);
			POINT pt = { LOWORD(lp), HIWORD(lp) };
			ScreenToClient(hParent, &pt); // to client coordinates relative to parent
			SendMessage(ChildWindowFromPoint(hParent, pt), // window below cursor
				WM_MOUSEWHEEL,
				MAKEWPARAM(LOWORD(wp) | 0x0800, HIWORD(wp)), // set 0x0800 bitflag and kick to window below cursor
				lp);
			return 0; // halt processing
		} else { // bitflag is set, WM_MOUSEWHEEL has been kicked here and can be safely processed
			wp &= ~0x0800; // unset bitflag
			break; // finally dispatch to default processing
		}
	case WM_NCDESTROY:
		RemoveWindowSubclass(hChild, _wheelHoverProc, idSubclass); // http://blogs.msdn.com/b/oldnewthing/archive/2003/11/11/55653.aspx
	}
	return DefSubclassProc(hChild, msg, wp, lp);
}

void sys::set_wheel_hover_behavior(HWND hParent)
{
	EnumChildWindows(hParent, [](HWND hChild, LPARAM lp)->BOOL {
		static UINT_PTR _uniqueSubclassId = 1;
		SetWindowSubclass(hChild, _wheelHoverProc, _uniqueSubclassId++,
			reinterpret_cast<DWORD_PTR>(GetParent(hChild))); // yes, subclass every control
		return TRUE;
	}, 0);
}

void sys::enable_x_button(HWND hWnd, bool enable)
{
	// Enable/disable the X button to close the window; has no effect on Alt+F4.
	HMENU hMenu = GetSystemMenu(hWnd, FALSE);
	if (hMenu) {
		UINT dwExtra = enable ? MF_ENABLED : (MF_DISABLED | MF_GRAYED);
		EnableMenuItem(hMenu, SC_CLOSE, MF_BYCOMMAND | dwExtra);
	}
}

bool sys::font_exists(const wchar_t* name)
{
	// http://cboard.cprogramming.com/windows-programming/90066-how-determine-if-font-support-unicode.html
	bool isInstalled = false;
	HDC hdc = GetDC(nullptr);
	EnumFontFamilies(hdc, name,
		(FONTENUMPROC)[](const LOGFONT* lpelf, const TEXTMETRIC *lpntm, DWORD fontType, LPARAM lp)->int {
		bool* pIsInstalled = reinterpret_cast<bool*>(lp);
		*pIsInstalled = true; // if we're here, font does exist
		return 0;
	}, reinterpret_cast<LPARAM>(&isInstalled));
	ReleaseDC(nullptr, hdc);
	return isInstalled;
}