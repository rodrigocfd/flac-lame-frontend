/**
 * Part of WOLF - WinAPI Object Lambda Framework
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/wolf
 */

#include "WindowProc.h"
using namespace wolf;
using std::wstring;

WindowProc::SetupProc::SetupProc()
	: hBackground(reinterpret_cast<HBRUSH>(COLOR_BTNFACE + 1)),
		hCursor(LoadCursor(nullptr, IDC_ARROW))
{
	// Useful background constants:
	// - window standard: COLOR_BTNFACE
	// - desktop: COLOR_DESKTOP
	// - black: COLOR_BTNTEXT
	// - white: COLOR_WINDOW
	// - grey: COLOR_APPWORKSPACE
}


WindowProc::~WindowProc()
{
}

ATOM WindowProc::_registerClass(HINSTANCE hInst, WNDCLASSEX& wc, SetupProc& setup)
{
	if (setup.className.empty()) {
		static unsigned int nextNum = 0;
		setup.className = L"WolfRandomWindow";
		setup.className += std::to_wstring(nextNum++); // will exhaust after 2^32-1 calls
	}

	wc.cbSize        = sizeof(WNDCLASSEX);
	wc.lpfnWndProc   = _proc;
	wc.hInstance     = hInst;
	wc.lpszClassName = setup.className.c_str();
	wc.hbrBackground = setup.hBackground;
	wc.hCursor       = setup.hCursor;
	wc.style         = CS_DBLCLKS;

	if (!wc.cbWndExtra) {
		wc.cbWndExtra = sizeof(WindowProc*); // pointer to class instance will be kept here
	}

	ATOM atom = RegisterClassEx(&wc);
	if (!atom) {
		DWORD lastError = GetLastError();
		if (lastError == ERROR_CLASS_ALREADY_EXISTS) {
			return static_cast<ATOM>(GetClassInfoEx(hInst, wc.lpszClassName, &wc)); // http://blogs.msdn.com/b/oldnewthing/archive/2004/10/11/240744.aspx
		}
		WindowMsgHandler::_errorShout(lastError, L"WindowProc::_registerClass", L"RegisterClassEx");
	}
	return atom;
}

LRESULT CALLBACK WindowProc::_proc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	// http://blogs.msdn.com/b/oldnewthing/archive/2014/02/03/10496248.aspx
	WindowProc *pSelf = nullptr;
	if (msg == WM_NCCREATE) {
		pSelf = reinterpret_cast<WindowProc*>((reinterpret_cast<CREATESTRUCT*>(lp))->lpCreateParams); // passed on CreateWindowEx()
		SetWindowLongPtr(hwnd, 0, reinterpret_cast<LONG_PTR>(pSelf)); // store pointer to object into HWND room
		pSelf->Window::operator=(hwnd); // store hWnd member
	} else {
		pSelf = reinterpret_cast<WindowProc*>(GetWindowLongPtr(hwnd, 0)); // from HWND room, zero if not set yet
	}
	return pSelf ?
		pSelf->WindowMsgHandler::_processMsg(msg, wp, lp) :
		DefWindowProc(hwnd, msg, wp, lp);
}