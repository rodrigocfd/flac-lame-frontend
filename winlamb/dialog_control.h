/**
 * Part of WinLamb - Windows Lambda Library
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winlamb
 */

#pragma once
#include "dialog.h"
#include "wnd_control.h"

/**
 * wnd <-- wnd_proc<traits_dialog> <-- dialog <-- dialog_control
 */

namespace winlamb {

class dialog_control : public dialog<>, public wnd_control {
public:
	virtual ~dialog_control() = default;
	dialog_control& operator=(const dialog_control&) = delete;

protected:
	dialog_control()
	{
		this->wnd_proc::on_message(WM_NCPAINT, [this](params p)->LRESULT {
			return this->wnd_control::_paint_themed_borders(p.wParam, p.lParam);
		});
	}

public:
	bool create(HWND hParent, int controlId, POINT position, SIZE size) override
	{
		// Dialog styles to be set on the resource editor:
		// - Control: true
		// - Style: child
		// - Visible: true

		if (this->wnd::hwnd()) {
			OutputDebugString(TEXT("ERROR: control dialog already created.\n"));
			return false;
		}

		HINSTANCE hInst = reinterpret_cast<HINSTANCE>(GetWindowLongPtr(hParent, GWLP_HINSTANCE));

		if (!CreateDialogParam(hInst, MAKEINTRESOURCE(this->dialog::setup.dialogId),
			hParent, wnd_proc::_process,
			reinterpret_cast<LPARAM>(static_cast<wnd_proc*>(this))) ) // _hWnd member is set on first message processing
		{
			OutputDebugString(TEXT("ERROR: control dialog not created, CreateDialogParam failed.\n"));
			return false;
		}

		SetWindowPos(this->wnd::hwnd(), nullptr,
			position.x, position.y,
			size.cx, size.cy, SWP_NOZORDER);
		return true;
	}

private:
	wnd_control::_paint_themed_borders;
};

}//namespace winlamb
