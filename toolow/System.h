//
// OS-related functions.
// Part of TOOLOW - Thin Object Oriented Layer Over Win32.
// @author Rodrigo Cesar de Freitas Dias
// @see https://github.com/rodrigocfd/toolow
//

#pragma once
#include "String.h"

class System final {
public:
	enum class Color { BUTTON=COLOR_BTNFACE, DESKTOP=COLOR_DESKTOP,
		BLACK=COLOR_BTNTEXT, WHITE=COLOR_WINDOW, GREY=COLOR_APPWORKSPACE };
	enum class Cursor { ARROW=32512, IBEAM=32513, CROSS=32515, HAND=32649, NO=32648,
		SIZEALL=32646, SIZENESW=32643, SIZENS=32645, SIZENWSE=32642, SIZEWE=32644 };

	inline static bool HasCtrl()  { return (::GetAsyncKeyState(VK_CONTROL) & 0x8000) != 0; }
	inline static bool HasShift() { return (::GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0; }

	static void  Thread(function<void()> callback);
	static DWORD Exec(const wchar_t *cmdLine);
	static DWORD Exec(const String& cmdLine) { return Exec(cmdLine.str()); }
	static DWORD ShellOpen(const wchar_t *file);
	static void  PopMenu(HWND hDlg, int popupMenuId, int x, int y, HWND hWndCoordsRelativeTo);
	static int   Rounds(float x);
	static int   Rounds(double x);
	static int   CeilMult(int n, int multiple);

	static String GetExePath();
	static String GetDesktopPath();
	static String GetMyDocsPath();
	static String GetRoamingPath();
};


// To use this macro, main window must have:
// int run(HINSTANCE hInst, LPWSTR cmdLine, int cmdShow);
#include <crtdbg.h>
#define RUN(AppClass) \
int WINAPI wWinMain(HINSTANCE hInst, HINSTANCE h0, LPWSTR cmdLine, int cmdShow) { \
	int ret = 0; \
	{	AppClass wnd; \
		ret = wnd.run(hInst, cmdLine, cmdShow); } \
	_ASSERT(!_CrtDumpMemoryLeaks()); \
	return ret; \
}