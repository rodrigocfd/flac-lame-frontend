/**
 * Part of WinLamb - Win32 API Lambda Library
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winlamb
 */

#pragma once
#include "base_inventory.h"
#include "base_threaded.h"

namespace wl {

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


class window final {
private:
	base_wnd _wnd;
public:
	base_inventory inventory;
	base_threaded threaded;
private:
	setup_window& _setup;

public:
	window(setup_window& setup) :
		threaded(_wnd, inventory, 0), _setup(setup) { }

	const base_wnd& wnd() const { return this->_wnd; }

	bool register_create(HWND hParent, HINSTANCE hInst = nullptr) {
		if (!this->_basic_initial_checks()) return false;
		if (!hParent && !hInst) return false;
		if (!hInst) hInst = reinterpret_cast<HINSTANCE>(GetWindowLongPtrW(hParent, GWLP_HINSTANCE));

		this->_setup.wndClassEx.cbSize = sizeof(WNDCLASSEX); // make sure of these
		this->_setup.wndClassEx.lpfnWndProc = _window_proc;
		this->_setup.wndClassEx.hInstance = hInst;

		ATOM atom = this->_register_class(hInst);
		if (!atom) return false;

		bool created = CreateWindowExW(this->_setup.exStyle, MAKEINTATOM(atom),
			this->_setup.title, this->_setup.style,
			this->_setup.position.x, this->_setup.position.y,
			this->_setup.size.cx, this->_setup.size.cy,
			hParent, this->_setup.menu, hInst,
			static_cast<LPVOID>(this)) != nullptr; // _hWnd member is set on first message processing

		if (!created) OutputDebugStringW(L"ERROR: CreateWindowEx failed.\n");
		return created;
	}

private:
	bool _basic_initial_checks() const {
		if (this->_wnd.hwnd()) {
			OutputDebugStringW(L"ERROR: tried to create window twice.\n");
			return false;
		}
		if (!this->_setup.wndClassEx.lpszClassName) {
			OutputDebugStringW(L"ERROR: window not created, no class name given.\n");
			return false;
		}
		return true;
	}

	ATOM _register_class(HINSTANCE hInst) {
		ATOM atom = RegisterClassExW(&this->_setup.wndClassEx);
		if (!atom) {
			if (GetLastError() == ERROR_CLASS_ALREADY_EXISTS) {
				atom = static_cast<ATOM>(GetClassInfoExW(hInst,
					this->_setup.wndClassEx.lpszClassName,
					&this->_setup.wndClassEx)); // https://blogs.msdn.microsoft.com/oldnewthing/20041011-00/?p=37603
			} else {
				OutputDebugStringW(L"ERROR: window not created, failed to register class ATOM.\n");
				return 0;
			}
		}
		return atom;
	}

	static LRESULT CALLBACK _window_proc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp) {
		window* pSelf = nullptr;

		if (msg == WM_NCCREATE) {
			pSelf = reinterpret_cast<window*>((reinterpret_cast<CREATESTRUCT*>(lp))->lpCreateParams);
			SetWindowLongPtrW(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pSelf));
			pSelf->_wnd = hWnd; // store HWND
		} else {
			pSelf = reinterpret_cast<window*>(GetWindowLongPtrW(hWnd, GWLP_USERDATA));
		}

		auto cleanupIfDestroyed = [&]()->void {
			if (msg == WM_NCDESTROY) {
				SetWindowLongPtrW(hWnd, GWLP_USERDATA, 0);
				if (pSelf) {
					pSelf->_wnd = nullptr;
				}
			}
		};

		if (pSelf) {
			params p = {msg, wp, lp};
			base_inventory::funcT* pFunc = pSelf->inventory.find_func(p);
			if (pFunc) {
				LRESULT ret = (*pFunc)(p);
				cleanupIfDestroyed();
				return ret;
			}
		}

		cleanupIfDestroyed();
		return DefWindowProcW(hWnd, msg, wp, lp);
	}
};

}//namespace wl