/**
 * Part of WinLamb - Windows Lambda Library
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winlamb
 */

#pragma once
#include "dialog.h"

/**
 * wnd <-- wnd_proc<traits_dialog> <-- dialog <-- dialog_modeless
 */

namespace winlamb {

class dialog_modeless : public dialog<> {
public:
	virtual ~dialog_modeless() = default;
	dialog_modeless& operator=(const dialog_modeless&) = delete;

protected:
	dialog_modeless()
	{
		this->wnd_proc::on_message(WM_CLOSE, [this](params p)->INT_PTR {
			DestroyWindow(this->wnd::hwnd());
			return TRUE;
		});
	}

public:
	void show(HWND hParent)
	{
		// To have dialog messages processed, pass HWND to parent's modeless_add().

		if (this->wnd::hwnd()) {
			OutputDebugString(TEXT("ERROR: modeless dialog already created.\n"));
			return;
		}

		HINSTANCE hInst = reinterpret_cast<HINSTANCE>(GetWindowLongPtr(hParent, GWLP_HINSTANCE));

		if (!CreateDialogParam(hInst, MAKEINTRESOURCE(this->dialog::setup.dialogId),
			hParent, wnd_proc::_process,
			reinterpret_cast<LPARAM>(static_cast<wnd_proc*>(this))) ) // _hWnd member is set on first message processing
		{
			OutputDebugString(TEXT("ERROR: modeless dialog not created, CreateDialogParam failed.\n"));
			return;
		}

		ShowWindow(this->wnd::hwnd(), SW_SHOW);
	}
};

}//namespace winlamb