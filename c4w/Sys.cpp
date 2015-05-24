/*!
 * OS-related stuff.
 * Part of C4W - Classes for Win32.
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/c4w
 */

#pragma warning(disable:4996) // GetVersionEx is deprecated for Win8.1, won't affect current behaviour
#include "Sys.h"
#include "Str.h"
#include <process.h>
#include <Shlobj.h>
#pragma comment(lib, "Shell32.lib") // SHGetFolderPath
using namespace c4w;
using std::function;
using std::wstring;

void sys::Thread(function<void()> callback)
{
	// Cheap alternative to std::thread([](){}).detach().

	struct CbPack { function<void()> cb; };
	CbPack *pack = new CbPack{ std::move(callback) };

	HANDLE thandle = reinterpret_cast<HANDLE>(_beginthreadex(nullptr, 0, [](void *ptr)->unsigned int {
		CbPack *pPack = reinterpret_cast<CbPack*>(ptr);
		pPack->cb(); // invoke user callback
		delete pPack;
		_endthreadex(0); // http://www.codeproject.com/Articles/7732/A-class-to-synchronise-thread-completions/
		return 0;
	}, pack, 0, nullptr));

	CloseHandle(thandle);
}

DWORD sys::Exec(const wchar_t *cmdLine)
{
	SECURITY_ATTRIBUTES sa = { 0 };
	sa.nLength = sizeof(sa);
	sa.bInheritHandle = TRUE;

	STARTUPINFO si = { 0 };
	si.cb = sizeof(si);
	si.dwFlags = STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_SHOW;

	// Avoid eventual crash under Unicode compiling.
	wchar_t *cmdLine2 = _wcsdup(cmdLine);

	PROCESS_INFORMATION pi = { 0 };
	DWORD dwExitCode = 1; // returned by executed program

	if (CreateProcess(nullptr, cmdLine2, &sa, nullptr, FALSE, 0, nullptr, nullptr, &si, &pi)) {
		WaitForSingleObject(pi.hProcess, INFINITE); // the program flow is stopped here to wait
		GetExitCodeProcess(pi.hProcess, &dwExitCode);
		CloseHandle(pi.hThread);
		CloseHandle(pi.hProcess);
	}

	free(cmdLine2);
	return dwExitCode;
}

void sys::PopMenu(HWND hDlg, int popupMenuId, int x, int y, HWND hWndCoordsRelativeTo)
{
	// Shows a popup context menu, anchored at the given coordinates.
	// The passed coordinates can be relative to any window.

	HMENU hMenu = LoadMenu(GetModuleHandle(nullptr), MAKEINTRESOURCE(popupMenuId));
	POINT ptDlg = { x, y }; // receives coordinates relative to hDlg
	ClientToScreen(hWndCoordsRelativeTo ? hWndCoordsRelativeTo : hDlg, &ptDlg); // to screen coordinates
	SetForegroundWindow(hDlg);
	TrackPopupMenu(GetSubMenu(hMenu, 0), 0, ptDlg.x, ptDlg.y, 0, hDlg, nullptr); // owned by dialog, so messages go to it
	PostMessage(hDlg, WM_NULL, 0, 0); // http://msdn.microsoft.com/en-us/library/ms648002%28VS.85%29.aspx
	DestroyMenu(hMenu);
}

wstring sys::GetExePath()
{
	wchar_t buf[MAX_PATH];
	GetModuleFileName(nullptr, buf, ARRAYSIZE(buf)); // retrieves EXE itself directory

	wstring ret = buf;
	ret.resize(str::FindRev(str::Sens::YES, ret, L'\\')); // truncate removing EXE filename and trailing backslash
#ifdef _DEBUG
	ret.resize(str::FindRev(str::Sens::YES, ret, L'\\')); // bypass "Debug" folder, remove trailing backslash too
#endif
	return ret;
}

wstring sys::GetDesktopPath()
{
	wchar_t buf[MAX_PATH];
	SHGetFolderPath(nullptr, CSIDL_DESKTOPDIRECTORY, nullptr, 0, buf); // won't have trailing backslash
	return buf;
}

wstring sys::GetMyDocsPath()
{
	wchar_t buf[MAX_PATH];
	SHGetFolderPath(nullptr, CSIDL_MYDOCUMENTS, nullptr, 0, buf); // won't have trailing backslash
	return buf;
}

wstring sys::GetRoamingPath()
{
	wchar_t buf[MAX_PATH];
	SHGetFolderPath(nullptr, CSIDL_APPDATA, nullptr, 0, buf); // won't have trailing backslash
	return buf;
}