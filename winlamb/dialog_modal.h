/**
 * Part of WinLamb - Windows Lambda Library
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winlamb
 */

#pragma once
#include "dialog.h"

/**
 * dialog_modal
 *  dialog
 *   wnd_proc<traits_dialog>
 *    wnd
 */

namespace winlamb {

class dialog_modal : public dialog<> {
public:
	virtual ~dialog_modal() = default;
	dialog_modal& operator=(const dialog_modal&) = delete;

protected:
	dialog_modal()
	{
		on_message(WM_CLOSE, [this](params p)->INT_PTR {
			EndDialog(hwnd(), IDOK);
			return TRUE;
		});
	}

public:
	int show(HWND hParent)
	{
		return static_cast<int>( DialogBoxParam(
			reinterpret_cast<HINSTANCE>(GetWindowLongPtr(hParent, GWLP_HINSTANCE)),
			MAKEINTRESOURCE(setup.dialogId), hParent, wnd_proc::_process,
			reinterpret_cast<LPARAM>(static_cast<wnd_proc*>(this))) ); // _hwnd member is set on first message processing
	}

protected:
	void center_on_parent()
	{
		RECT rc = { 0 }, rcParent = { 0 };
		GetWindowRect(hwnd(), &rc);
		GetWindowRect(GetParent(hwnd()), &rcParent); // both relative to screen
		SetWindowPos(hwnd(), nullptr,
			rcParent.left + (rcParent.right - rcParent.left)/2 - (rc.right - rc.left)/2,
			rcParent.top + (rcParent.bottom - rcParent.top)/2 - (rc.bottom - rc.top)/2,
			0, 0, SWP_NOZORDER | SWP_NOSIZE);
	}

private:
	wnd_proc<traits_dialog>::_process;
};

}//namespace winlamb
