/**
 * Part of WinLamb - Win32 API Lambda Library
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winlamb
 */

#pragma once
#include "inventory.h"
#include "threaded.h"

namespace wl {
namespace internals {

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


template<typename setupT>
class window final {
private:
	base_wnd _wnd;
public:
	inventory inventoryMsg;
	threaded  threader;
	setupT    setup;

	window() : threader(_wnd, inventoryMsg, 0) { }

	const base_wnd& wnd() const { return this->_wnd; }

	bool register_create(HWND hParent, HINSTANCE hInst = nullptr) {
		if (!this->_basic_initial_checks()) return false;
		if (!hParent && !hInst) return false;
		if (!hInst) hInst = reinterpret_cast<HINSTANCE>(GetWindowLongPtrW(hParent, GWLP_HINSTANCE));

		this->setup.wndClassEx.cbSize = sizeof(WNDCLASSEX); // make sure of these
		this->setup.wndClassEx.lpfnWndProc = _window_proc;
		this->setup.wndClassEx.hInstance = hInst;

		ATOM atom = _register_class(hInst);
		if (!atom) return false;

		bool created = CreateWindowExW(this->setup.exStyle, MAKEINTATOM(atom),
			this->setup.title, this->setup.style,
			this->setup.position.x, this->setup.position.y,
			this->setup.size.cx, this->setup.size.cy,
			hParent, this->setup.menu, hInst,
			static_cast<LPVOID>(this)) != nullptr;

		if (!created) OutputDebugStringW(L"ERROR: CreateWindowEx failed.\n");
		return created;
	}

private:
	bool _basic_initial_checks() const {
		if (this->_wnd.hwnd()) {
			OutputDebugStringW(L"ERROR: tried to create window twice.\n");
			return false;
		}
		if (!this->setup.wndClassEx.lpszClassName) {
			OutputDebugStringW(L"ERROR: window not created, no class name given.\n");
			return false;
		}
		return true;
	}

	ATOM _register_class(HINSTANCE hInst) {
		ATOM atom = RegisterClassExW(&setup.wndClassEx);
		if (!atom) {
			if (GetLastError() == ERROR_CLASS_ALREADY_EXISTS) {
				atom = static_cast<ATOM>(GetClassInfoExW(hInst,
					this->setup.wndClassEx.lpszClassName,
					&this->setup.wndClassEx)); // https://blogs.msdn.microsoft.com/oldnewthing/20041011-00/?p=37603
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
			inventory::funcT* pFunc = pSelf->inventoryMsg.find_func(p);
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

}//namespace internals
}//namespace wl