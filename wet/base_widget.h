/**
 * Part of WET - WinAPI Easy Toolkit
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/wet
 */

#pragma once
#include "base_wnd.h"

/**
 * base_wnd <-- base_widget
 */

namespace wet {

template<typename derivedT>
class base_widget : public base_wnd {
public:
	virtual ~base_widget()                = default;
	base_widget()                         = default;
	base_widget(HWND hWnd)                { this->operator=(hWnd); }
	base_widget(const base_widget& other) { this->operator=(other); }

	derivedT& operator=(HWND hWnd) {
		this->base_wnd::operator=(hWnd);
		return this->_derived();
	}

	derivedT& operator=(const base_widget& other) {
		return this->operator=(other.hwnd());
	}

	virtual derivedT& be(const base_wnd* parent, int controlId) {
		return this->operator=(GetDlgItem(parent->hwnd(), controlId));
	}

	derivedT& create(const base_wnd* parent, int controlId, const wchar_t* caption,
		POINT pos, SIZE size, const wchar_t* className,
		DWORD styles = (WS_CHILD | WS_VISIBLE), DWORD exStyles = 0)
	{
		if (this->base_wnd::hwnd()) {
			DBG(L"ERROR: base_widget already created.\n");
			return this->_derived();
		}

		return this->operator=( CreateWindowExW(exStyles, className, caption, styles,
			pos.x, pos.y, size.cx, size.cy, parent->hwnd(),
			reinterpret_cast<HMENU>(static_cast<UINT_PTR>(controlId)),
			parent->hinstance(), nullptr) );
	}

	derivedT& focus() const {
		SetFocus(this->base_wnd::hwnd());
		return this->_derived();
	}

	bool has_focus() const {
		GetFocus() == this->base_wnd::hwnd();
	}

	derivedT& enable(bool doEnable) const {
		EnableWindow(this->base_wnd::hwnd(), doEnable);
		return this->_derived();
	}

	bool is_enabled() const {
		return IsWindowEnabled(this->base_wnd::hwnd());
	}

private:
	derivedT& _derived()       { return static_cast<derivedT&>(*this); }
	derivedT& _derived() const { return const_cast<derivedT&>(static_cast<const derivedT&>(*this)); }
};

}//namespace wet