/**
 * Part of WinLamb - Windows Lambda Library
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winlamb
 */

#pragma once
#include "wnd_proc.h"
#include "traits_window.h"

/**
 * wnd <-- wnd_proc<traits_window> <-- window
 */

namespace winlamb {

struct setup_window {
	WNDCLASSEX     wndClassEx;
	const wchar_t* title;
	DWORD          style, exStyle;
	POINT          position;
	SIZE           size;
	HMENU          menu;
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
		if (!this->setup.wndClassEx.lpszClassName) {
			OutputDebugString(TEXT("ERROR: window not created, no class name given.\n"));
			return false;
		}

		if (this->wnd::hwnd()) {
			OutputDebugString(TEXT("ERROR: tried to create window twice.\n"));
			return false;
		}

		if (!hParent && !hInst) return false;
		if (!hInst) hInst = reinterpret_cast<HINSTANCE>(GetWindowLongPtr(hParent, GWLP_HINSTANCE));

		this->setup.wndClassEx.cbSize = sizeof(WNDCLASSEX); // make sure of these
		this->setup.wndClassEx.lpfnWndProc = wnd_proc::_process;
		this->setup.wndClassEx.hInstance = hInst;

		ATOM atom = this->_register_class(hInst);
		if (!atom) return false;

		return CreateWindowEx(this->setup.exStyle, MAKEINTATOM(atom),
			this->setup.title, this->setup.style,
			this->setup.position.x, this->setup.position.y,
			this->setup.size.cx, this->setup.size.cy,
			hParent, this->setup.menu, hInst,
			static_cast<LPVOID>(static_cast<wnd_proc*>(this))) != nullptr; // _hwnd member is set on first message processing
	}

private:
	ATOM _register_class(HINSTANCE hInst)
	{
		ATOM atom = RegisterClassEx(&this->setup.wndClassEx);
		if (!atom) {
			if (GetLastError() == ERROR_CLASS_ALREADY_EXISTS) {
				atom = static_cast<ATOM>(GetClassInfoEx(hInst,
					this->setup.wndClassEx.lpszClassName,
					&this->setup.wndClassEx)); // https://blogs.msdn.microsoft.com/oldnewthing/20041011-00/?p=37603
			} else {
				OutputDebugString(TEXT("ERROR: window not created, failed to register class ATOM.\n"));
				return 0;
			}
		}
		return atom;
	}

	wnd_proc<traits_window>::_process;
};

}//namespace winlamb