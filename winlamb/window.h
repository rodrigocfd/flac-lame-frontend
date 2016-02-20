/**
 * Part of WinLamb - Windows Lambda Library
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winlamb
 */

#pragma once
#include "wnd_proc.h"
#include "traits_window.h"

/**
 * window
 *  wnd_proc<traits_window>
 *   wnd
 */

namespace winlamb {

struct setup_window {
	WNDCLASSEX     wcx;
	const wchar_t *title;
	DWORD          style, exStyle;
	POINT          position;
	SIZE           size;
	HMENU          menu;	
	setup_window() : wcx({ 0 }), title(nullptr), style(0), exStyle(0),
		position({0,0}), size({0,0}), menu(nullptr) { }
};


template<typename setupT = setup_window>
class window : virtual public wnd_proc<traits_window> {
public:
	setupT setup;
	virtual ~window() = default;

	bool create(HWND hParent, HINSTANCE hInst = nullptr)
	{
		if (!setup.wcx.lpszClassName) {
			OutputDebugString(L"ERROR: window not created, no class name given.\n");
			return false;
		}

		if (hwnd()) return false; // window already created
		if (!hParent && !hInst) return false;
		if (!hInst) hInst = reinterpret_cast<HINSTANCE>(GetWindowLongPtr(hParent, GWLP_HINSTANCE));

		setup.wcx.cbSize = sizeof(WNDCLASSEX); // make sure of these
		setup.wcx.lpfnWndProc = wnd_proc::_process;
		setup.wcx.hInstance = hInst;

		ATOM atom = RegisterClassEx(&setup.wcx);
		if (!atom) {
			if (GetLastError() == ERROR_CLASS_ALREADY_EXISTS) {
				atom = static_cast<ATOM>(GetClassInfoEx(hInst,
					setup.wcx.lpszClassName, &setup.wcx)); // https://blogs.msdn.microsoft.com/oldnewthing/20041011-00/?p=37603
			} else {
				return false;
			}
		}

		return CreateWindowEx(setup.exStyle, MAKEINTATOM(atom), setup.title, setup.style,
			setup.position.x, setup.position.y, setup.size.cx, setup.size.cy,
			hParent, setup.menu, hInst,
			static_cast<LPVOID>(static_cast<wnd_proc*>(this))) != nullptr; // _hwnd member is set on first message processing
	}

private:
	wnd_proc<traits_window>::_process;
};

}//namespace winlamb