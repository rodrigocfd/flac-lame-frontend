/**
 * Part of WinLamb - Windows Lambda Library
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winlamb
 */

#pragma once
#include "threaded.h"
#include "traits_dialog.h"
#include "font.h"

/**
 * dialog
 *  threaded<traits_dialog>
 *   proc<traits_dialog>
 *    handle
 */

namespace winlamb {

struct setup_dialog {
	int dialogId;
	setup_dialog() : dialogId(0) { }
};


template<typename setupT = setup_dialog>
class dialog : public threaded<traits_dialog> {
private:
	msg_func_type _userInitDialog;
public:
	setupT setup;
	virtual ~dialog() = default;

	dialog()
	{
		proc::on_message(WM_INITDIALOG, [this](WPARAM wp, LPARAM lp)->INT_PTR {
			font::set_ui_on_children(hwnd()); // if user creates more controls, font must be set manually on them
			return _userInitDialog ? _userInitDialog(wp, lp) : TRUE;
		});
	}

	virtual void on_message(UINT msg, msg_func_type callback) override
	{
		if (msg == WM_INITDIALOG) _userInitDialog = std::move(callback);
		else proc::on_message(msg, std::move(callback));
	}
};

}//namespace winlamb