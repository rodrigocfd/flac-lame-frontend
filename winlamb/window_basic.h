/**
 * Part of WinLamb - Windows Lambda Library
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winlamb
 */

#pragma once
#include "window_thread.h"
#include "window_traits.h"

 /**
  * window_basic
  *  window_thread<window_basic_traits>
  *   window_proc<window_basic_traits>
  *    window
  */

namespace winlamb {

class window_basic : public window_thread<window_basic_traits> {
public:
	struct setup_type {
		WNDCLASSEX     wcx;
		const wchar_t *title;
		DWORD          style, exStyle;
		POINT          position;
		SIZE           size;
		HMENU          menu;
		HACCEL         accelTable;
		setup_type() : wcx({ 0 }), title(nullptr), style(0), exStyle(0),
			position({0,0}), size({0,0}), menu(nullptr), accelTable(nullptr) { }
	};

	setup_type setup;
	virtual ~window_basic() = default;

	bool create(HWND hParent, HINSTANCE hInst = nullptr)
	{
		if (hwnd()) return false; // window already created
		if (!hParent && !hInst) return false;
		if (!hInst) hInst = reinterpret_cast<HINSTANCE>(GetWindowLongPtr(hParent, GWLP_HINSTANCE));

		setup.wcx.cbSize = sizeof(WNDCLASSEX); // make sure of these
		setup.wcx.lpfnWndProc = window_proc::_proc;
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
			static_cast<LPVOID>(this)) != nullptr; // _hwnd member is set on first message processing
	}

private:
	window_proc::_proc;
};

}//namespace winlamb