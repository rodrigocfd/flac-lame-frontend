/**
 * Part of WET - WinAPI Easy Toolkit
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/wet
 */

#pragma once
#include "base_widget_text.h"
#include "subclass.h"

/**
 * base_wnd <-- base_widget <-- base_widget_text <-- textbox
 */

namespace wet {

class textbox final : public base_widget_text<textbox> {
public:
	struct selection final {
		int start;
		int len;
	};

private:
	subclass _subclass;
	std::function<void(BYTE)> _onKeyUp;

public:
	virtual ~textbox() = default;
	textbox()          = default;
	textbox(HWND hWnd) { this->operator=(hWnd); }

	textbox& operator=(HWND hWnd) {
		this->base_widget::operator=(hWnd);
		this->_subclass.remove_subclass();
		this->_subclass.install_subclass(hWnd,
			[&](params p)->LRESULT { return this->_txtproc(p); });
		return *this;
	}

	virtual textbox& be(const base_wnd* parent, int controlId) override {
		this->base_widget::be(parent, controlId);
		return this->operator=(this->hwnd()); // force subclass
	}

	textbox& create(const base_wnd* parent, int controlId, POINT pos, LONG width) {
		return this->_raw_create(parent, controlId, pos, {width,21}, ES_AUTOHSCROLL);
	}

	textbox& create_password(const base_wnd* parent, int id, POINT pos, LONG width) {
		return this->_raw_create(parent, id, pos, {width,21}, ES_AUTOHSCROLL | ES_PASSWORD);
	}

	textbox& create_multi_line(const base_wnd* parent, int controlId, POINT pos, SIZE size) {
		return this->_raw_create(parent, controlId, pos, size, ES_MULTILINE | ES_WANTRETURN);
	}

	textbox& textbox::selection_set(selection selec) {
		SendMessageW(this->base_wnd::hwnd(), EM_SETSEL, selec.start, selec.start + selec.len);
		return *this;
	}

	textbox& selection_set_all() {
		return this->selection_set({0, -1});
	}

	selection selection_get() const {
		int p0 = 0, p1 = 0;
		SendMessageW(this->base_wnd::hwnd(), EM_GETSEL,
			reinterpret_cast<WPARAM>(&p0), reinterpret_cast<LPARAM>(&p1));
		return { p0, p1 - p0 }; // start, length
	}

	textbox& selection_replace(const wchar_t* t) {
		SendMessageW(this->base_wnd::hwnd(), EM_REPLACESEL,
			TRUE, reinterpret_cast<LPARAM>(t));
		return *this;
	}

	textbox& selection_replace(const std::wstring& t) {
		return this->selection_replace(t.c_str());
	}

	textbox& on_key_up(std::function<void(BYTE)> callback) {
		this->_onKeyUp = std::move(callback);
		return *this;
	}

private:
	textbox& _raw_create(const base_wnd* parent, int controlId, POINT pos, SIZE size, DWORD extraStyles) {
		this->base_widget::create(parent, controlId, nullptr,
			pos, size, L"Edit",
			WS_CHILD | WS_VISIBLE | extraStyles,
			WS_EX_CLIENTEDGE);
		return this->operator=(this->hwnd()); // force subclass
	}

	LRESULT _txtproc(params p) {
		if (p.message == WM_KEYDOWN) {
			if (LOWORD(p.wParam == VK_ESCAPE)) { // ESC http://www.williamwilling.com/blog/?p=28
				SendMessageW(GetAncestor(this->base_wnd::hwnd(), GA_PARENT), WM_COMMAND,
					IDCANCEL, reinterpret_cast<LPARAM>(this->base_wnd::hwnd()));
				return 0;
			}
		} else if (p.message == WM_GETDLGCODE) {
			bool hasCtrl = (GetAsyncKeyState(VK_CONTROL) & 0x8000) != 0;
			if (p.lParam && p.wParam == 'A' && hasCtrl) { // Ctrl+A to select all text
				p.wParam = 0; // prevent propagation, therefore beep
				SendMessageW(this->base_wnd::hwnd(), EM_SETSEL, 0, -1);
				return DLGC_WANTCHARS;
			}
		} else if (p.message == WM_KEYUP && this->_onKeyUp) {
			this->_onKeyUp(static_cast<BYTE>(p.wParam));
			return 0;
		}
		return this->_subclass.def_proc(p);
	}

	base_widget::create;
};

}//namespace wet