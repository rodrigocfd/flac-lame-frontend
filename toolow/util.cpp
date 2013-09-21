
#include <math.h>
#include "util.h"
#include "Shlobj.h"
#pragma comment(lib, "Shell32.lib")
#pragma warning(disable:4996) // _itow()

void debug(const wchar_t *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	
	String buf;
	buf.fmtv(fmt, args);
	
	va_end(args);
	OutputDebugString(buf.str());
}

Ptr<String> fmt(const wchar_t *fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	Ptr<String> s = new String(); // a smart pointer will be returned
	s->fmtv(fmt, args);

	va_end(args);
	return s;
}

int round(float x)
{
	return (int)floor(x + 0.5); // <math.h>
}

int round(double x)
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

void explodeMultiStr(const wchar_t *multiStr, Array<String> *pBuf)
{
	// Example multiStr:
	// L"first one\0second one\0third one\0"
	// Assumes a well-formed multiStr.

	// Count number of null-delimited strings; string end with double null.
	int numStrings = 0;
	const wchar_t *pRun = multiStr;
	while(*pRun) {
		++numStrings;
		pRun += lstrlen(pRun) + 1;
	}

	// Alloc array of strings.
	pBuf->realloc(numStrings);

	// Copy each string.	
	pRun = multiStr;
	for(int i = 0; i < numStrings; ++i) {
		(*pBuf)[i] = pRun;
		pRun += lstrlen(pRun) + 1;
	}
}

void explodeQuotedStr(const wchar_t *quotedStr, Array<String> *pBuf)
{
	// Example quotedStr:
	// "First one" NoQuoteSecond "Third one"

	// Count number of strings.
	int numStrings = 0;
	const wchar_t *pRun = quotedStr;
	while(*pRun) {
		if(*pRun == L'\"') { // begin of quoted string
			++pRun; // point to 1st char of string
			for(;;) {
				if(!*pRun) {
					break; // won't compute open-quoted
				} else if(*pRun == L'\"') {
					++pRun; // point to 1st char after closing quote
					++numStrings;
					break;
				}
				++pRun;
			}
		} else if(!iswspace(*pRun)) { // 1st char of non-quoted string
			++pRun; // point to 2nd char of string
			while(*pRun && !iswspace(*pRun) && *pRun != L'\"') ++pRun; // passed string
			++numStrings;
		} else {
			++pRun; // some white space
		}
	}

	// Alloc array of strings.
	pBuf->realloc(numStrings);

	// Alloc and copy each string.
	pRun = quotedStr;
	const wchar_t *pBase;
	int i = 0;
	while(*pRun) {
		String& theStr = (*pBuf)[i]; // current buffer string being worked upon

		if(*pRun == L'\"') { // begin of quoted string
			++pRun; // point to 1st char of string
			pBase = pRun;
			for(;;) {
				if(!*pRun) {
					break; // won't compute open-quoted
				} else if(*pRun == L'\"') {
					int len = (int)(pRun - pBase);
					theStr.reserve(len);
					memcpy(theStr.ptrAt(0), pBase, sizeof(wchar_t) * len); // copy to buffer
					++i; // next string

					++pRun; // point to 1st char after closing quote
					break;
				}
				++pRun;
			}
		} else if(!iswspace(*pRun)) { // 1st char of non-quoted string
			pBase = pRun;
			++pRun; // point to 2nd char of string
			while(*pRun && !iswspace(*pRun) && *pRun != L'\"') ++pRun; // passed string
			
			int len = (int)(pRun - pBase);
			theStr.reserve(len);
			memcpy(theStr.ptrAt(0), pBase, sizeof(wchar_t) * len); // copy to buffer
			++i; // next string
		} else {
			++pRun; // some white space
		}
	}
}

String* GetPathTo::Exe(String *pBuf)
{
	pBuf->reserve(MAX_PATH);
	(*pBuf) = L"";

	GetModuleFileName(0, pBuf->ptrAt(0), pBuf->reserved() + 1); // retrieves EXE itself directory
	(*pBuf)[ pBuf->findr(L'\\') ] = L'\0'; // truncate removing EXE file name

#ifdef _DEBUG
	(*pBuf)[ pBuf->findr(L'\\') ] = L'\0'; // bypass "Debug" folder	
#endif

	return pBuf; // return same passed buffer, won't have trailing backslash
}

String* GetPathTo::_Folder(String *pBuf, BYTE cslid_id)
{
	pBuf->reserve(MAX_PATH);
	(*pBuf) = L"";

	// This ugly pseudo-enum is to avoid #inclusion of Shobj.h into util.h.
	const int cslids[] = { CSIDL_DESKTOPDIRECTORY, CSIDL_MYDOCUMENTS, CSIDL_APPDATA };

	SHGetFolderPath(0, cslids[cslid_id], 0, 0, pBuf->ptrAt(0)); // won't have trailing backslash
	return pBuf; // return same passed buffer
}