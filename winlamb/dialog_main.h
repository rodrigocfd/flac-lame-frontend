/**
 * Part of WinLamb - Windows Lambda Library
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winlamb
 */

#pragma once
#include "dialog.h"
#include "wnd_main.h"
#include "wnd_loop.h"

/**
*                        +--- wnd_msgs <---+
*        +-- wnd_proc <--+                 +-- dialog <--+
*        |               +-- wnd_thread <--+             |
* wnd <--+               |                               +-- dialog_main
*        |               +--- wnd_main <-----------------+
*        |                                               |
*        +------------------- wnd_loop <-----------------+
 */

namespace winlamb {

struct setup_dialog_main final : public setup_dialog {
	int iconId;
	int accelTableId;
	setup_dialog_main() : iconId(0), accelTableId(0) { }
};


class dialog_main :
	public dialog<setup_dialog_main>,
	public wnd_main<traits_dialog>,
	public wnd_loop
{
public:
	virtual ~dialog_main() = default;
	dialog_main& operator=(const dialog_main&) = delete;

protected:
	dialog_main()
	{
		this->wnd_proc::on_message(WM_CLOSE, [this](params p)->INT_PTR {
			DestroyWindow(this->wnd::hwnd());
			return TRUE;
		});
	}

public:
	int run(HINSTANCE hInst, int cmdShow) override
	{
		InitCommonControls();

		if (!this->dialog::setup.dialogId) {
			OutputDebugString(TEXT("ERROR: main dialog not created, no dialog ID given.\n"));
			return -1;
		}

		HWND hwndRet = CreateDialogParam(hInst, MAKEINTRESOURCE(this->dialog::setup.dialogId),
			nullptr, wnd_proc::_process,
			reinterpret_cast<LPARAM>(static_cast<wnd_proc*>(this)) ); // _hwnd member is set on first message processing
		if (!hwndRet) {
			OutputDebugString(TEXT("ERROR: main dialog not created, CreateDialogParam failed.\n"));
			return -1;
		}

		HACCEL hAccel = nullptr;
		if (this->dialog::setup.accelTableId) {
			hAccel = LoadAccelerators(hInst, MAKEINTRESOURCE(this->dialog::setup.accelTableId));
		}

		this->_set_icon(hInst);
		ShowWindow(this->wnd::hwnd(), cmdShow);
		return this->wnd_loop::_msg_loop(hAccel); // this can be used as program return value
	}

private:
	void _set_icon(HINSTANCE hInst)
	{
		if (this->dialog::setup.iconId) {
			SendMessage(this->wnd::hwnd(), WM_SETICON, ICON_SMALL,
				reinterpret_cast<LPARAM>(reinterpret_cast<HICON>(LoadImage(hInst,
					MAKEINTRESOURCE(this->dialog::setup.iconId),
					IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR))));
			SendMessage(hwnd(), WM_SETICON, ICON_BIG,
				reinterpret_cast<LPARAM>(reinterpret_cast<HICON>(LoadImage(hInst,
					MAKEINTRESOURCE(this->dialog::setup.iconId),
					IMAGE_ICON, 32, 32, LR_DEFAULTCOLOR))));
		}
	}

	wnd_proc<traits_dialog>::_process;
	wnd_loop::_msg_loop;
};

}//namespace winlamb
