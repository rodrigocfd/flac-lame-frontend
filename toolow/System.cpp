//
// System-related functions.
// Part of TOOLOW - Thin Object Oriented Layer Over Win32.
// @author Rodrigo Cesar de Freitas Dias
// @see https://github.com/rodrigocfd/toolow
//

#include "System.h"
#include <math.h>
#include <process.h>
#include <Shlobj.h>
#pragma comment(lib, "Shell32.lib") // SHGetFolderPath

void System::Thread(function<void()> callback)
{
	struct CbPack { function<void()> cb; };
	CbPack *pack = new CbPack{ MOVE(callback) };

	HANDLE thandle = (HANDLE)_beginthreadex(nullptr, 0, [](void *ptr)->unsigned int {
		CbPack *pPack = (CbPack*)ptr;
		pPack->cb(); // invoke user callback
		delete pPack;
		_endthreadex(0); // http://www.codeproject.com/Articles/7732/A-class-to-synchronise-thread-completions/
		return 0;
	}, pack, 0, nullptr);

	CloseHandle(thandle);
}

DWORD System::Exec(const wchar_t *cmdLine)
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

	if(CreateProcess(0, cmdLine2, &sa, 0, FALSE, 0, 0, 0, &si, &pi)) {
		WaitForSingleObject(pi.hProcess, INFINITE); // the program flow is stopped here to wait
		GetExitCodeProcess(pi.hProcess, &dwExitCode);
		CloseHandle(pi.hThread);
		CloseHandle(pi.hProcess);
	}

	free(cmdLine2);
	return dwExitCode;
}

DWORD System::ShellOpen(const wchar_t *file)
{
	return (DWORD)ShellExecute(0, L"open", file, 0, 0, SW_SHOWNORMAL);
}

void System::PopMenu(HWND hDlg, int popupMenuId, int x, int y, HWND hWndCoordsRelativeTo)
{
	// Shows a popup context menu, anchored at the given coordinates.
	// The passed coordinates can be relative to any window.

	HMENU hMenu = LoadMenu(GetModuleHandle(0), MAKEINTRESOURCE(popupMenuId));
	POINT ptDlg = { x, y }; // receives coordinates relative to hDlg
	ClientToScreen(hWndCoordsRelativeTo ? hWndCoordsRelativeTo : hDlg, &ptDlg); // to screen coordinates
	SetForegroundWindow(hDlg);
	TrackPopupMenu(GetSubMenu(hMenu, 0), 0, ptDlg.x, ptDlg.y, 0, hDlg, 0); // owned by dialog, so messages go to it
	PostMessage(hDlg, WM_NULL, 0, 0); // http://msdn.microsoft.com/en-us/library/ms648002%28VS.85%29.aspx
	DestroyMenu(hMenu);
}

int Rounds(float x)
{
	return (int)floor(x + 0.5); // <math.h>
}

int Rounds(double x)
{
	return (int)floor(x + 0.5); // <math.h>
}

int CeilMult(int n, int multiple)
{
	// Ceil up to next multiple of.
	// http://stackoverflow.com/questions/3407012/c-rounding-up-to-the-nearest-multiple-of-a-number
	if(!multiple) return n;
	int remainder = n % multiple;
	if(!remainder) return n;
	return n + multiple - remainder - (n < 0 ? multiple : 0); // bugfix for negative numbers
}

String System::GetExePath()
{
	String ret;
	ret.reserve(MAX_PATH);

	GetModuleFileName(nullptr, ret.ptrAt(0), ret.reserved() + 1); // retrieves EXE itself directory
	ret[ ret.findrCS(L'\\') ] = L'\0'; // truncate removing EXE file name, remove trailing backslash

#ifdef _DEBUG
	ret[ ret.findrCS(L'\\') ] = L'\0'; // bypass "Debug" folder, remove trailing backslash
#endif

	return ret;
}

String System::GetDesktopPath()
{
	String ret;
	ret.reserve(MAX_PATH);
	SHGetFolderPath(nullptr, CSIDL_DESKTOPDIRECTORY, nullptr, 0, ret.ptrAt(0)); // won't have trailing backslash
	return ret;
}

String System::GetMyDocsPath()
{
	String ret;
	ret.reserve(MAX_PATH);
	SHGetFolderPath(nullptr, CSIDL_MYDOCUMENTS, nullptr, 0, ret.ptrAt(0)); // won't have trailing backslash
	return ret;
}

String System::GetRoamingPath()
{
	String ret;
	ret.reserve(MAX_PATH);
	SHGetFolderPath(nullptr, CSIDL_APPDATA, nullptr, 0, ret.ptrAt(0)); // won't have trailing backslash
	return ret;
}