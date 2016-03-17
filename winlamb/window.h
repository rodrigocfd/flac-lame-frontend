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
	WNDCLASSEX   wndClassEx;
	const TCHAR *title;
	DWORD        style, exStyle;
	POINT        position;
	SIZE         size;
	HMENU        menu;
	setup_window() : wndClassEx({ 0 }), title(nullptr), style(0), exStyle(0),
		position({0,0}), size({0,0}), menu(nullptr) { }
};


template<typename setupT = setup_window>
class window : virtual public wnd_proc<traits_window> {
public:
	setupT setup;
	virtual ~window() = default;

protected:
	window() = default;

public:
	bool create(HWND hParent, HINSTANCE hInst = nullptr)
	{
		if (!setup.wndClassEx.lpszClassName) {
			OutputDebugString(L"ERROR: window not created, no class name given.\n");
			return false;
		}

		if (hwnd()) {
			OutputDebugString(L"ERROR: tried to create window twice.\n");
			return false;
		}

		if (!hParent && !hInst) return false;
		if (!hInst) hInst = reinterpret_cast<HINSTANCE>(GetWindowLongPtr(hParent, GWLP_HINSTANCE));

		setup.wndClassEx.cbSize = sizeof(WNDCLASSEX); // make sure of these
		setup.wndClassEx.lpfnWndProc = wnd_proc::_process;
		setup.wndClassEx.hInstance = hInst;

		ATOM atom = _register_class(hInst);
		if (!atom) return false;

		return CreateWindowEx(setup.exStyle, MAKEINTATOM(atom), setup.title, setup.style,
			setup.position.x, setup.position.y, setup.size.cx, setup.size.cy,
			hParent, setup.menu, hInst,
			static_cast<LPVOID>(static_cast<wnd_proc*>(this))) != nullptr; // _hwnd member is set on first message processing
	}

private:
	ATOM _register_class(HINSTANCE hInst)
	{
		ATOM atom = RegisterClassEx(&setup.wndClassEx);
		if (!atom) {
			if (GetLastError() == ERROR_CLASS_ALREADY_EXISTS) {
				atom = static_cast<ATOM>(GetClassInfoEx(hInst,
					setup.wndClassEx.lpszClassName, &setup.wndClassEx)); // https://blogs.msdn.microsoft.com/oldnewthing/20041011-00/?p=37603
			} else {
				OutputDebugString(L"ERROR: window not created, failed to register class ATOM.\n");
				return 0;
			}
		}
		return atom;
	}

	wnd_proc<traits_window>::_process;
};

}//namespace winlamb