
#include "Dialog.h"

DialogModal::~DialogModal()
{
}

int DialogModal::show(Window *parent, int dialogId, int accelTableId)
{
	return (int)DialogBoxParam(parent->getInstance(),
		MAKEINTRESOURCE(dialogId), parent->hWnd(), Dialog::_DialogProc,
		(LPARAM)this); // pass pointer to class instance
}

INT_PTR DialogModal::msgHandler(UINT msg, WPARAM wp, LPARAM lp)
{
	switch(msg)
	{
	case WM_INITDIALOG:
		{
			RECT rcPop = { 0 }, rcParent = { 0 };
			this->getWindowRect(&rcPop);
			this->getParent().getWindowRect(&rcParent); // all relative to screen
			this->setPos(0,
				rcParent.left + (rcParent.right - rcParent.left) / 2 - (rcPop.right - rcPop.left) / 2,
				rcParent.top + (rcParent.bottom - rcParent.top) / 2 - (rcPop.bottom - rcPop.top) / 2,
				0, 0, SWP_NOZORDER | SWP_NOSIZE); // center dialog on parent
		}
		break;
	case WM_CLOSE:
		this->endDialog(0);
		return TRUE;
	}
	return DialogPopup::msgHandler(msg, wp, lp); // forward to parent class message handler
}