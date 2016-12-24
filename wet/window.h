/**
 * Part of WET - WinAPI Easy Toolkit
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/wet
 */

#pragma once
#include "base_wnd_thread.h"

/**
 * base_wnd <-- base_wnd_loop <-- base_wnd_thread <-- window
 */

namespace wet {

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
class window : public base_wnd_thread {
public:
	virtual ~window() = default;

protected:
	window() = default;
	setupT setup;
	virtual LRESULT proc(params p) = 0;

	LRESULT def_proc(params p) const {
		switch (p.message) {
		case base_wnd_thread::WM_THREAD_MESSAGE:
			this->base_wnd_thread::_process_thread(p);
			return 0;
		}
		return DefWindowProcW(this->base_wnd::hwnd(), p.message, p.wParam, p.lParam);
	}

	bool _register_create(HWND hParent, HINSTANCE hInst = nullptr) {
		if (!this->_basic_check()) return false;
		if (!hParent && !hInst) return false;
		if (!hInst) hInst = reinterpret_cast<HINSTANCE>(GetWindowLongPtrW(hParent, GWLP_HINSTANCE));

		this->setup.wndClassEx.cbSize = sizeof(WNDCLASSEX); // make sure of these
		this->setup.wndClassEx.lpfnWndProc = _proc;
		this->setup.wndClassEx.hInstance = hInst;

		ATOM atom = this->_register_class(hInst);
		if (!atom) return false;

		bool created = CreateWindowExW(this->setup.exStyle, MAKEINTATOM(atom),
			this->setup.title, this->setup.style,
			this->setup.position.x, this->setup.position.y,
			this->setup.size.cx, this->setup.size.cy,
			hParent, this->setup.menu, hInst,
			static_cast<LPVOID>(this)) != nullptr; // _hWnd member is set on first message processing

		if (!created) DBG(L"ERROR: CreateWindowEx failed.\n");
		return created;
	}

private:
	ATOM _register_class(HINSTANCE hInst) {
		ATOM atom = RegisterClassExW(&this->setup.wndClassEx);
		if (!atom) {
			if (GetLastError() == ERROR_CLASS_ALREADY_EXISTS) {
				atom = static_cast<ATOM>(GetClassInfoExW(hInst,
					this->setup.wndClassEx.lpszClassName,
					&this->setup.wndClassEx)); // https://blogs.msdn.microsoft.com/oldnewthing/20041011-00/?p=37603
			} else {
				DBG(L"ERROR: window not created, failed to register class ATOM.\n");
				return 0;
			}
		}
		return atom;
	}

	bool _basic_check() const {
		if (this->base_wnd::hwnd()) {
			DBG(L"ERROR: tried to create window twice.\n");
			return false;
		}
		if (!this->setup.wndClassEx.lpszClassName) {
			DBG(L"ERROR: window not created, no class name given.\n");
			return false;
		}
		return true;
	}

	static LRESULT CALLBACK _proc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp) {
		window* pSelf = nullptr;
		
		if (msg == WM_NCCREATE) {
			pSelf = reinterpret_cast<window*>((reinterpret_cast<CREATESTRUCT*>(lp))->lpCreateParams);
			SetWindowLongPtrW(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pSelf));
			pSelf->base_wnd::operator=(hWnd); // store HWND
		} else {
			pSelf = reinterpret_cast<window*>(GetWindowLongPtrW(hWnd, GWLP_USERDATA));
		}

		auto cleanupIfDestroyed = [&]()->void {
			if (msg == WM_NCDESTROY) {
				SetWindowLongPtrW(hWnd, GWLP_USERDATA, 0);
				if (pSelf) {
					pSelf->base_wnd::operator=(nullptr);
				}
			}
		};

		if (pSelf) {
			LRESULT ret = pSelf->proc({ msg, wp, lp });
			cleanupIfDestroyed();
			return ret;
		}
		
		cleanupIfDestroyed();
		return DefWindowProcW(hWnd, msg, wp, lp);
	}

	base_wnd::_text;
	base_wnd_thread::_process_thread;
};

}//namespace wet