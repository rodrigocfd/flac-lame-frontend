/**
 * Part of WinLamb - Windows Lambda Library
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winlamb
 */

#pragma once
#include "window_dialog.h"

 /**
  * window_dialog_modal
  *  window_dialog
  *   window_thread<window_dialog_traits>
  *    window_proc<window_dialog_traits>
  *     window
  */

namespace winlamb {

class window_dialog_modal : public window_dialog {
public:
	virtual ~window_dialog_modal() = default;
	window_dialog_modal& operator=(const window_dialog_modal&) = delete;

	window_dialog_modal()
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
			MAKEINTRESOURCE(setup.dialogId), hParent, window_proc::_proc,
			reinterpret_cast<LPARAM>(this)) ); // _hwnd member is set on first message processing
	}

private:
	window_proc::_proc;
};

}//namespace winlamb
