
#include "Frame.h"

FrameCtrl::~FrameCtrl()
{
}

void FrameCtrl::create(int id, Window *parent, ATOM atom, int x, int y, int cx, int cy,
	FrameCtrl::Style::Border border, FrameCtrl::Style::Container container,
	FrameCtrl::Style::Scroll scroll, FrameCtrl::Style::TabStop tabStop)
{
	CreateWindowEx(border | container, // http://blogs.msdn.com/b/oldnewthing/archive/2004/07/30/201988.aspx
		(LPCWSTR)MAKELONG(atom, 0), 0,
		CS_DBLCLKS | WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | scroll | tabStop,
		x, y, cx, cy,
		parent->hWnd(), (HMENU)id, parent->getInstance(),
		(LPVOID)this); // pass pointer to object; hWnd is set on WM_NCCREATE
}

LRESULT FrameCtrl::msgHandler(UINT msg, WPARAM wp, LPARAM lp)
{
	switch(msg)
	{
	case WM_NCPAINT:
		if(this->_drawBorders(wp, lp)) // themed borders
			return 0;
		break;
	}
	return Frame::msgHandler(msg, wp, lp); // forward to parent class message handler
}