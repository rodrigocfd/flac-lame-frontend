/**
 * Part of WinLamb - Windows Lambda Library
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winlamb
 */

#pragma once
#include "dialog.h"

/**
 *                     +--- wnd_msgs <----+
 * wnd <-- wnd_proc <--+                  +-- dialog <-- dialog_modal
 *                     +-- wnd_thread <---+
 */

namespace winlamb {

class dialog_modal : public dialog<> {
public:
	virtual ~dialog_modal() = default;
	dialog_modal& operator=(const dialog_modal&) = delete;

protected:
	dialog_modal()
	{
		this->wnd_proc::on_message(WM_CLOSE, [this](params p)->INT_PTR {
			EndDialog(this->wnd::hwnd(), IDOK);
			return TRUE;
		});
	}

public:
	int show(HWND hParent)
	{
		return static_cast<int>(DialogBoxParam(
			reinterpret_cast<HINSTANCE>(GetWindowLongPtr(hParent, GWLP_HINSTANCE)),
			MAKEINTRESOURCE(this->dialog::setup.dialogId), hParent, wnd_proc::_process,
			reinterpret_cast<LPARAM>(static_cast<wnd_proc*>(this)) )); // _hwnd member is set on first message processing
	}

protected:
	void center_on_parent()
	{
		RECT rc = { 0 }, rcParent = { 0 };
		GetWindowRect(this->wnd::hwnd(), &rc);
		GetWindowRect(GetParent(this->wnd::hwnd()), &rcParent); // both relative to screen
		SetWindowPos(this->wnd::hwnd(), nullptr,
			rcParent.left + (rcParent.right - rcParent.left)/2 - (rc.right - rc.left)/2,
			rcParent.top + (rcParent.bottom - rcParent.top)/2 - (rc.bottom - rc.top)/2,
			0, 0, SWP_NOZORDER | SWP_NOSIZE);
	}

private:
	wnd_proc<traits_dialog>::_process;
};

}//namespace winlamb
