/**
 * Part of WinLamb - Win32 API Lambda Library
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winlamb
 */

#pragma once
#include "internals/base_wnd.h"
#include "internals/i_inventory.h"
#include <CommCtrl.h>

namespace wl {

class subclass final : public internals::i_inventory {
public:
	using funcT = std::function<LRESULT(params)>;

private:
	internals::base_wnd  _wnd;
	internals::inventory _inventory;
	UINT                 _subclassId;

public:
	~subclass() { this->remove_subclass(); }

	subclass() : i_inventory(_inventory) { }

	void remove_subclass() {
		if (this->_wnd.hwnd()) {
			RemoveWindowSubclass(this->_wnd.hwnd(), _proc, this->_subclassId);
			this->_wnd = nullptr;
		}
	}

	void install_subclass(HWND hWnd) {
		this->remove_subclass();
		this->_wnd = hWnd;
		this->_subclassId = _next_id();
		SetWindowSubclass(this->_wnd.hwnd(), _proc, this->_subclassId,
			reinterpret_cast<DWORD_PTR>(this));
	}

private:
	static LRESULT CALLBACK _proc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp,
		UINT_PTR idSubclass, DWORD_PTR refData)
	{
		subclass* pSelf = reinterpret_cast<subclass*>(refData);
		if (pSelf && pSelf->_wnd.hwnd()) {
			params p = {msg, wp, lp};
			internals::inventory::funcT* func = pSelf->_inventory.find_func(p);
			if (func) {
				LRESULT ret = (*func)(p);
				if (msg == WM_NCDESTROY) {
					pSelf->remove_subclass();
				}
				return ret;
			}
		}

		if (msg == WM_NCDESTROY) {
			RemoveWindowSubclass(hWnd, _proc, idSubclass);
		}
		return DefSubclassProc(hWnd, msg, wp, lp);
	}

	static UINT _next_id() {
		static UINT firstId = 0;
		return firstId++;
	}
};

}//namespace wl