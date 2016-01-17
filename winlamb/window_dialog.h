/**
 * Part of WinLamb - Windows Lambda Library
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winlamb
 */

#pragma once
#include "window_thread.h"
#include "window_traits.h"
#include "window_font.h"

/**
 * window_dialog
 *  window_thread<window_dialog_traits>
 *   window_proc<window_dialog_traits>
 *    window
 */

namespace winlamb {

class window_dialog : public window_thread<window_dialog_traits> {
private:
	msg_func_type _userInitDialog;
public:
	struct setup_type {
		int dialogId;
		int iconId;
		int accelTableId;
		setup_type() : dialogId(0), iconId(0), accelTableId(0) { }
	};

	setup_type setup;
	virtual ~window_dialog() = default;

	window_dialog()
	{
		window_proc::on_message(WM_INITDIALOG, [this](WPARAM wp, LPARAM lp)->INT_PTR {
			window_font::set_ui_on_children(hwnd()); // if user creates controls, font must be set manually
			return _userInitDialog ? _userInitDialog(wp, lp) : TRUE;
		});
	}

	virtual void on_message(UINT msg, msg_func_type callback) override
	{
		if (msg == WM_INITDIALOG) _userInitDialog = std::move(callback);
		else window_proc::on_message(msg, std::move(callback));
	}
};

}//namespace winlamb