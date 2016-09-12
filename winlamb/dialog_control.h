/**
 * Part of WinLamb - Windows Lambda Library
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winlamb
 */

#pragma once
#include "dialog.h"
#include "wnd_control.h"

/**
 *                     +--- wnd_msgs <----+
 *                     |                  +-- dialog <--+
 * wnd <-- wnd_proc <--+-- wnd_thread <---+             +-- dialog_control
 *                     |                                |
 *                     +-- wnd_control <----------------+
 */

namespace winlamb {

class dialog_control :
	public dialog<>,
	public wnd_control<traits_dialog>
{
public:
	virtual ~dialog_control() = default;
	dialog_control& operator=(const dialog_control&) = delete;

protected:
	dialog_control() = default;

public:
	bool create(HWND hParent, int controlId, POINT position, SIZE size) override
	{
		// Dialog styles to be set on the resource editor:
		// - Control: true
		// - Style: child
		// - Visible: true (otherwise will start invisible)
		// - Client Edge: true (if you want a border, will add WS_EX_CLIENTEDGE)

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

		this->_check_bad_styles();
		SetWindowPos(this->wnd::hwnd(), nullptr,
			position.x, position.y,
			size.cx, size.cy, SWP_NOZORDER);
		return true;
	}

private:
	void _check_bad_styles()
	{
		DWORD style = GetWindowLongPtr(this->wnd::hwnd(), GWL_STYLE);
		if (!(style & DS_CONTROL)) {
			OutputDebugString(TEXT("ERROR: control template doesn't have DS_CONTROL style.\n"));
		}
		if (!(style & WS_CHILD)) {
			OutputDebugString(TEXT("ERROR: control template doesn't have WS_CHILD style.\n"));
		}
	}
};

}//namespace winlamb
