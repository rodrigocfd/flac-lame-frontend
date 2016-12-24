/**
 * Part of WET - WinAPI Easy Toolkit
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/wet
 */

#pragma once
#include <functional>
#include "base_wnd.h"
#include "params.h"

namespace wet {

class subclass final {
private:
	HWND _hOwner;
	UINT _subclassId;
	std::function<LRESULT(params)> _callback;

public:
	~subclass() { this->remove_subclass(); }
	subclass() : _hOwner(nullptr) { }

	LRESULT def_proc(params p) const {
		return DefSubclassProc(this->_hOwner, p.message, p.wParam, p.lParam);
	}

	void install_subclass(HWND hWnd, std::function<LRESULT(params)> callback) {
		this->remove_subclass();
		this->_hOwner = hWnd;
		this->_subclassId = _next_id();
		this->_callback = std::move(callback);		
		SetWindowSubclass(this->_hOwner, _proc, this->_subclassId,
			reinterpret_cast<DWORD_PTR>(this));
	}

	void install_subclass(const base_wnd* owner, std::function<LRESULT(params)> callback) {
		return this->install_subclass(owner->hwnd(), std::move(callback));
	}

	void remove_subclass() {
		RemoveWindowSubclass(this->_hOwner, _proc, this->_subclassId);
		this->_hOwner = nullptr;
	}

private:
	static LRESULT CALLBACK _proc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp,
		UINT_PTR idSubclass, DWORD_PTR refData)
	{
		subclass* pSelf = reinterpret_cast<subclass*>(refData);
		if (pSelf && pSelf->_hOwner) {
			return pSelf->_callback({ msg, wp, lp });
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

}//namespace wet