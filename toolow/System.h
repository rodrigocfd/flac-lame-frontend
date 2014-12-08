//
// OS-related functions.
// Part of WOLF - Win32 Object Lambda Framework.
// @author Rodrigo Cesar de Freitas Dias
// @see https://github.com/rodrigocfd/wolf
//

#pragma once
#include "String.h"

struct System final {
	enum class Color { BUTTON=COLOR_BTNFACE, DESKTOP=COLOR_DESKTOP,
		BLACK=COLOR_BTNTEXT, WHITE=COLOR_WINDOW, GREY=COLOR_APPWORKSPACE };
	enum class Cursor { ARROW=32512, IBEAM=32513, CROSS=32515, HAND=32649, NO=32648,
		SIZEALL=32646, SIZENESW=32643, SIZENS=32645, SIZENWSE=32642, SIZEWE=32644 };

	inline static bool HasCtrl()  { return (::GetAsyncKeyState(VK_CONTROL) & 0x8000) != 0; }
	inline static bool HasShift() { return (::GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0; }

	static void   Thread(function<void()> callback);
	static DWORD  Exec(const wchar_t *cmdLine);
	static DWORD  Exec(const String& cmdLine) { return Exec(cmdLine.str()); }
	static DWORD  ShellOpen(const wchar_t *file);
	static void   PopMenu(HWND hDlg, int popupMenuId, int x, int y, HWND hWndCoordsRelativeTo);
	static String GetExePath();
	static String GetDesktopPath();
	static String GetMyDocsPath();
	static String GetRoamingPath();

	class Math final {
	public:
		static int Rounds(float x);
		static int Rounds(double x);
		static int CeilMult(int n, int multiple);
	};

	class Date final {
	private:
		SYSTEMTIME _st;
	public:
		Date()                              { setNow(); }
		explicit Date(LONGLONG ms)          { setFromMs(ms); }
		explicit Date(const SYSTEMTIME *st) { setFromSt(st); }
		explicit Date(const FILETIME *ft)   { setFromFt(ft); }
		Date&             setNow();
		Date&             setFromSt(const SYSTEMTIME *st) { ::memcpy(&_st, st, sizeof(SYSTEMTIME)); return *this; }
		Date&             setFromMs(LONGLONG ms);
		Date&             setFromFt(const FILETIME *ft);
		const SYSTEMTIME& get() const                     { return _st; }
		LONGLONG          getTimestamp() const;
		LONGLONG          minus(const Date& other) const;
		Date&             addMs(LONGLONG ms);
		Date&             addSec(LONGLONG sec)            { return addMs(sec * 1000); }
		Date&             addMin(LONGLONG min)            { return addSec(min * 60); }
		Date&             addHour(LONGLONG h)             { return addMin(h * 60); }
		Date&             addDay(LONGLONG d)              { return addHour(d * 24); }
	private:
		static void _StToLi(const SYSTEMTIME& st, LARGE_INTEGER& li);
		static void _LiToSt(const LARGE_INTEGER& li, SYSTEMTIME& st);
	};
};

// To use this macro, your main window class must have:
// int run(HINSTANCE hInst, int cmdShow);
#include <crtdbg.h>
#define RUN(AppClass) \
int WINAPI wWinMain(HINSTANCE hInst, HINSTANCE h0, LPWSTR cmdLine, int cmdShow) { \
	int ret = 0; \
	{	AppClass wnd; \
		ret = wnd.run(hInst, cmdShow); } \
	_ASSERT(!_CrtDumpMemoryLeaks()); \
	return ret; \
}