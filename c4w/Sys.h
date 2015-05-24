/*!
 * OS-related stuff.
 * Part of C4W - Classes for Win32.
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/c4w
 */

#pragma once
#include <functional>
#include <string>
#include <Windows.h>

// WinMain automation for *App classes.
#include <crtdbg.h>
#define RUN(AppClass) \
int WINAPI wWinMain(HINSTANCE hInst, HINSTANCE h0, LPWSTR cmdLine, int cmdShow) { \
	int ret = 0; \
	{	AppClass wnd; \
		ret = wnd.run(hInst, cmdShow); } \
	_ASSERT(!_CrtDumpMemoryLeaks()); \
	return ret; }


namespace c4w {

namespace sys {
	enum class Color { BUTTON=COLOR_BTNFACE, DESKTOP=COLOR_DESKTOP,
		BLACK=COLOR_BTNTEXT, WHITE=COLOR_WINDOW, GREY=COLOR_APPWORKSPACE };
	enum class Cursor { ARROW=32512, IBEAM=32513, CROSS=32515, HAND=32649, NO=32648,
		SIZEALL=32646, SIZENESW=32643, SIZENS=32645, SIZENWSE=32642, SIZEWE=32644 };

	inline bool  HasCtrl()       { return (::GetAsyncKeyState(VK_CONTROL) & 0x8000) != 0; }
	inline bool  HasShift()      { return (::GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0; }
	inline SIZE  GetScreenSize() { return { ::GetSystemMetrics(SM_CXSCREEN), ::GetSystemMetrics(SM_CYSCREEN) }; }
	void         Thread(std::function<void()> callback);
	DWORD        Exec(const wchar_t *cmdLine);
	inline DWORD Exec(const std::wstring& cmdLine) { return Exec(cmdLine.c_str()); }
	void         PopMenu(HWND hDlg, int popupMenuId, int x, int y, HWND hWndCoordsRelativeTo);
	std::wstring GetExePath();
	std::wstring GetDesktopPath();
	std::wstring GetMyDocsPath();
	std::wstring GetRoamingPath();
}//namespace sys

}//namespace c4w