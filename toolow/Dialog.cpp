
#include "Dialog.h"

Dialog::~Dialog()
{
}

INT_PTR Dialog::msgHandler(UINT msg, WPARAM wp, LPARAM lp)
{
	return FALSE; // default-most message processing for a dialog
}

INT_PTR CALLBACK Dialog::_DialogProc(HWND hdlg, UINT msg, WPARAM wp, LPARAM lp)
{
	Dialog *pSelf; // in run-time, will be a pointer to the derived-most class
	if(msg == WM_INITDIALOG) {
		pSelf = (Dialog*)lp; // passed on CreateDialogParam() or DialogBoxParam()
		SetWindowLongPtr(hdlg, GWLP_USERDATA, (LONG_PTR)pSelf); // store pointer to object into HWND room
		*((Window*)pSelf) = hdlg; // assign hWnd member
	}
	else pSelf = (Dialog*)GetWindowLongPtr(hdlg, GWLP_USERDATA); // from HWND room, zero if not set yet
	return pSelf ? pSelf->msgHandler(msg, wp, lp) : FALSE; // works, since msgHandler() is virtual
}