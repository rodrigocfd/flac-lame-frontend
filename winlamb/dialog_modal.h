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
 *   proc<traits_dialog>
 *    wnd
 */

namespace winlamb {

class dialog_modal : public dialog<> {
public:
	virtual ~dialog_modal() = default;
	dialog_modal& operator=(const dialog_modal&) = delete;

	dialog_modal()
	{
		on_message(WM_CLOSE, [this](WPARAM wp, LPARAM lp)->INT_PTR {
			EndDialog(hwnd(), IDOK);
			return TRUE;
		});
	}

	int show(HWND hParent)
	{
		return static_cast<int>( DialogBoxParam(
			reinterpret_cast<HINSTANCE>(GetWindowLongPtr(hParent, GWLP_HINSTANCE)),
			MAKEINTRESOURCE(setup.dialogId), hParent, wnd_proc::_process,
			reinterpret_cast<LPARAM>(static_cast<wnd_proc*>(this))) ); // _hwnd member is set on first message processing
	}

private:
	wnd_proc<traits_dialog>::_process;
};

}//namespace winlamb
