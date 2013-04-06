
#include "Dialog.h"

DialogCtrl::~DialogCtrl()
{
}

void DialogCtrl::create(int id, Window *parent, int x, int y, int cx, int cy)
{
	// Dialog styles to be set on the resource editor:
	// - Control: true
	// - Style: child
	// - Visible: true
	CreateDialogParam(parent->getInstance(), MAKEINTRESOURCE(id), parent->hWnd(),
		Dialog::_DialogProc, (LPARAM)this); // pass pointer to object
	this->setPos(0, x, y, cx, cy, SWP_NOZORDER);
}

INT_PTR DialogCtrl::msgHandler(UINT msg, WPARAM wp, LPARAM lp)
{
	switch(msg)
	{
	case WM_NCPAINT:
		if(this->_drawBorders(wp, lp)) // themed borders
			return TRUE;
		break;
	}
	return Dialog::msgHandler(msg, wp, lp); // forward to parent class message handler
}