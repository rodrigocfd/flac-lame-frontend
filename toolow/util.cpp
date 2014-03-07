
#include <math.h>
#include "util.h"

int rounds(float x)
{
	return (int)floor(x + 0.5); // <math.h>
}

int rounds(double x)
{
	return (int)floor(x + 0.5); // <math.h>
}

int ceilmult(int n, int multiple)
{
	// Ceil up to next multiple of.
	// http://stackoverflow.com/questions/3407012/c-rounding-up-to-the-nearest-multiple-of-a-number
	if(!multiple) return n;
	int remainder = n % multiple;
	if(!remainder) return n;
	return n + multiple - remainder - (n < 0 ? multiple : 0); // bugfix for negative numbers
}

int indexOfBin(const BYTE *pData, int dataLen, const wchar_t *what, bool asWideChar)
{
	// Returns the position of a string within a binary data block, if present.

	int whatlen = lstrlen(what);
	int pWhatSz = whatlen * (asWideChar ? 2 : 1);
	BYTE *pWhat = (BYTE*)_alloca(pWhatSz * sizeof(BYTE));
	if(asWideChar) {
		memcpy(pWhat, what, whatlen * sizeof(wchar_t)); // simply copy the wide string, each char+zero
	} else {
		for(int i = 0; i < whatlen; ++i)
			pWhat[i] = LOBYTE(what[i]); // raw conversion from wchar_t to char
	}

	for(int i = 0; i < dataLen; ++i)
		if(!memcmp(pData + i, pWhat, pWhatSz * sizeof(BYTE)))
			return i;
	
	return -1; // not found
}

DWORD exec(const wchar_t *cmdLine)
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

DWORD shellOpen(const wchar_t *file)
{
	return (DWORD)ShellExecute(0, L"open", file, 0, 0, SW_SHOWNORMAL);
}

void popMenu(HWND hDlg, int popupMenuId, int x, int y, HWND hWndCoordsRelativeTo)
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