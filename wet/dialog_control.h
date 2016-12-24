/**
 * Part of WET - WinAPI Easy Toolkit
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/wet
 */

#pragma once
#include "dialog.h"
#include "base_wnd_control.h"

/**
 *             +-- base_wnd_loop <-- base_wnd_thread <-- dialog --+
 * base_wnd <--+                                                  +-- dialog_control
 *             +---------------- base_wnd_control <---------------+
 */

namespace wet {

class dialog_control :
	public dialog<>,
	public base_wnd_control
{
protected:
	dialog_control() = default;

	INT_PTR def_proc(params p) const {
		switch (p.message) {
		case WM_NCPAINT:
			return this->base_wnd_control::_paint_themed_borders(p.wParam, p.lParam);
		}
		return dialog::def_proc(p);
	}

public:
	virtual ~dialog_control() = default;
	dialog_control& operator=(const dialog_control&) = delete;

	virtual bool create(HWND hParent, int controlId, POINT position, SIZE size) override {
		// Dialog styles to be set on the resource editor:
		// - Border: none
		// - Control: true
		// - Style: child
		// - Visible: true (otherwise will start invisible)
		// - Client Edge: true (if you want a border, will add WS_EX_CLIENTEDGE)

		if (!this->dialog::_basic_check()) return false;

		if (!CreateDialogParamW(
			reinterpret_cast<HINSTANCE>(GetWindowLongPtrW(hParent, GWLP_HINSTANCE)),
			MAKEINTRESOURCE(this->dialog::setup.dialogId),
			hParent, dialog::_proc,
			reinterpret_cast<LPARAM>(static_cast<dialog*>(this))) ) // _hWnd member is set on first message processing
		{
			DBG(L"ERROR: control dialog not created, CreateDialogParam failed.\n");
			return false;
		}

		this->_check_bad_styles();
		SetWindowLongPtrW(this->base_wnd::hwnd(), GWLP_ID, controlId);
		SetWindowPos(this->base_wnd::hwnd(), nullptr,
			position.x, position.y,
			size.cx, size.cy, SWP_NOZORDER);
		return true;
	}

	virtual bool create(const base_wnd* parent, int controlId, POINT position, SIZE size) override {
		return this->create(parent->hwnd(), controlId, position, size);
	}

private:
	void _check_bad_styles() {
		DWORD style = static_cast<DWORD>(GetWindowLongPtrW(this->base_wnd::hwnd(), GWL_STYLE));
		if (!(style & DS_CONTROL)) {
			DBG(L"ERROR: control template doesn't have DS_CONTROL style.\n");
		}
		if (!(style & WS_CHILD)) {
			DBG(L"ERROR: control template doesn't have WS_CHILD style.\n");
		}
	}

	base_wnd_loop::_msg_loop;
	base_wnd_control::_paint_themed_borders;
	dialog::_basic_check;
	dialog::_proc;
	dialog::def_proc;
};

}//namespace wet