
#include "Splitter.h"
#include "DeviceContext.h"

Splitter::~Splitter()
{
}

void Splitter::create(HWND hLeft, HWND hRight, int id, Window *parent, int x, int y, int cx, int cy)
{
	static ATOM atom = 0;
	if(!atom) atom = Frame::Register(L"splitterwnd");

	this->_hLeft = hLeft;
	this->_hRight = hRight;
	this->_isDrag = false;

	RECT rc = { 0 };
	if(!cx || !cy) parent->getClientRect(&rc); // adjust to fit parent?
	FrameCtrl::create(id, parent, atom, x, y,
		cx ? cx : rc.right, cy ? cy : rc.bottom,
		FrameCtrl::Style::NOBORDER, FrameCtrl::Style::CONTAINER, FrameCtrl::Style::NOSCROLL, FrameCtrl::Style::TABSTOP);
}

LRESULT Splitter::msgHandler(UINT msg, WPARAM wp, LPARAM lp)
{
	switch(msg)
	{
	case WM_ERASEBKGND:
		return 0;
	case WM_SIZE:
		{
			RECT rc = { 0 };
			this->getParent().getClientRect(&rc);
			this->getParent().sendMessage(WM_SIZE, wp, MAKELPARAM(rc.right, rc.bottom));
		}
		return 0;
	case WM_LBUTTONUP:
		if(this->_isDrag) { // if user was dragging, then he just stopped
			::ReleaseCapture();
			this->_isDrag = false;
		}
		return 0;
	}
	return FrameCtrl::msgHandler(msg, wp, lp);
}

void VSplitter::create(HWND hLeft, HWND hRight, int id, Window *parent, int x, int y, int cx, int cy)
{
	RECT rc = { 0 };
	parent->getClientRect(&rc);
	this->_pos = rc.right / 2 - _Metrics::GRIPWIDTH / 2; // start position in the middle of the window
	Splitter::create(hLeft, hRight, id, parent, x, y, cx, cy);
}

LRESULT VSplitter::msgHandler(UINT msg, WPARAM wp, LPARAM lp)
{
	switch(msg)
	{
	case WM_PAINT:
		{
			DCBuffered dc(this->hWnd());
			dc.drawEdge(this->_pos + _Metrics::GRIPWIDTH / 2, 0, 2, dc.cy - 2, EDGE_ETCHED, BF_LEFT);
		}
		return 0;

	case WM_GETMINMAXINFO:
		{
			RECT rcC = { 0 }, rcW = { 0 };
			this->getClientRect(&rcC);
			this->getWindowRect(&rcW);
			((MINMAXINFO*)lp)->ptMinTrackSize.x = // limit horizontal size
				this->_pos + _Metrics::GRIPWIDTH + _Metrics::OFFSET + (rcW.right - rcW.left - rcC.right);
		}
		return 0;

	case WM_SIZE:
		{
			int state = (int)wp;
			int cx = LOWORD(lp);
			int cy = HIWORD(lp);

			if(state != SIZE_MINIMIZED) { // if not minimized
				if(this->_pos > cx - _Metrics::PADDING - _Metrics::OFFSET && cx > _Metrics::OFFSET * 2)
					this->_pos = cx - _Metrics::PADDING - _Metrics::OFFSET; // keep min width of right panel

				this->invalidateRect(true);
				HDWP hdwp = ::BeginDeferWindowPos(2);

				POINT pos = { _Metrics::PADDING, _Metrics::PADDING };
				this->clientToScreen(&pos);
				this->getParent().screenToClient(&pos);
				::DeferWindowPos(hdwp, this->_hLeft, 0, pos.x, pos.y, // x,y relative to parent
					this->_pos - _Metrics::PADDING * 2, cy - _Metrics::PADDING * 2, SWP_NOZORDER); // cx,cy

				pos.x = this->_pos + _Metrics::GRIPWIDTH + _Metrics::PADDING;
				pos.y = _Metrics::PADDING;
				this->clientToScreen(&pos);
				this->getParent().screenToClient(&pos);
				::DeferWindowPos(hdwp, this->_hRight, 0, pos.x, pos.y, // x,y relative to parent
					cx - this->_pos - _Metrics::GRIPWIDTH - _Metrics::PADDING * 2, cy - _Metrics::PADDING * 2, // cx,cy
					SWP_NOZORDER);

				::EndDeferWindowPos(hdwp);
			}
		}
		break;

	case WM_LBUTTONDOWN:
		{
			int x = LOWORD(lp); // x-coordinate of cursor
			if(x >= this->_pos && x <= this->_pos + _Metrics::GRIPWIDTH) { // if over splitter grip
				::SetCursor(::LoadCursor(0, IDC_SIZEWE));
				::SetCapture(this->hWnd());
				this->_isDrag = true; // we're dragging now
			}
		}
		return 0;

	case WM_MOUSEMOVE:
		{
			RECT rc = { 0 };
			this->getClientRect(&rc);
			int x = LOWORD(lp); // client area cursor coordinate

			if(this->_isDrag && (x > _Metrics::OFFSET && x < rc.right - _Metrics::OFFSET - _Metrics::GRIPWIDTH)) {
				this->_pos = x; // splitter grip position follows the cursor position
				this->sendMessage(WM_SIZE, SIZE_RESTORED, MAKELPARAM(rc.right, rc.bottom)); // will resize both panels
			} else if(x >= this->_pos && x <= this->_pos + _Metrics::GRIPWIDTH) {
				// Every time cursor is over the splitter grip, set the cursor.
				// While dragging, the cursor is already set by WM_LBUTTONDOWN,
				// so we don't need to set it again here.
				::SetCursor(::LoadCursor(0, IDC_SIZEWE));
			}
		}
		return 0;
	}
	return Splitter::msgHandler(msg, wp, lp);
}

void HSplitter::create(HWND hLeft, HWND hRight, int id, Window *parent, int x, int y, int cx, int cy)
{
	RECT rc = { 0 };
	parent->getClientRect(&rc);
	this->_pos = rc.bottom / 2 - _Metrics::GRIPWIDTH / 2; // start position in the middle of window
	Splitter::create(hLeft, hRight, id, parent, x, y, cx, cy);
}

LRESULT HSplitter::msgHandler(UINT msg, WPARAM wp, LPARAM lp)
{
	switch(msg)
	{
	case WM_PAINT:
		{
			DCBuffered dc(this->hWnd());
			dc.drawEdge(2, this->_pos + _Metrics::GRIPWIDTH / 2, dc.cx - 2, 0, EDGE_ETCHED, BF_TOP);
		}
		return 0;

	case WM_GETMINMAXINFO:
		{
			RECT rcC = { 0 }, rcW = { 0 };
			this->getClientRect(&rcC);
			this->getWindowRect(&rcW);
			((MINMAXINFO*)lp)->ptMinTrackSize.y = // limit vertical size
				this->_pos + _Metrics::GRIPWIDTH + _Metrics::OFFSET + (rcW.bottom - rcW.top - rcC.bottom);
		}
		return 0;

	case WM_SIZE:
		{
			int state = (int)wp;
			int cx = LOWORD(lp);
			int cy = HIWORD(lp);

			if(state != SIZE_MINIMIZED) { // if not minimized
				if(this->_pos > cy - _Metrics::PADDING - _Metrics::OFFSET && cy > _Metrics::OFFSET * 2)
					this->_pos = cy - _Metrics::PADDING - _Metrics::OFFSET; // keep min height of bottom panel

				this->invalidateRect(true);
				HDWP hdwp = ::BeginDeferWindowPos(2);

				POINT pos = { _Metrics::PADDING, _Metrics::PADDING };
				this->clientToScreen(&pos);
				this->getParent().screenToClient(&pos);
				::DeferWindowPos(hdwp, this->_hLeft, 0, pos.x, pos.y, // x,y relative to parent
					cx - _Metrics::PADDING * 2, this->_pos - _Metrics::PADDING * 2, SWP_NOZORDER); // cx,cy

				pos.x = _Metrics::PADDING;
				pos.y = this->_pos + _Metrics::GRIPWIDTH + _Metrics::PADDING;
				this->clientToScreen(&pos);
				this->getParent().screenToClient(&pos);
				::DeferWindowPos(hdwp, this->_hRight, 0, pos.x, pos.y, // x,y relative to parent
					cx - _Metrics::PADDING * 2, cy - this->_pos - _Metrics::GRIPWIDTH - _Metrics::PADDING * 2, // cx,cy
					SWP_NOZORDER);

				::EndDeferWindowPos(hdwp);
			}
		}
		break;

	case WM_LBUTTONDOWN:
		{
			int y = HIWORD(lp); // y-coordinate of cursor
			if(y >= this->_pos && y <= this->_pos + _Metrics::GRIPWIDTH) { // if over splitter grip
				::SetCursor(LoadCursor(0, IDC_SIZENS));
				::SetCapture(this->hWnd());
				this->_isDrag = true; // we're dragging now
			}
		}
		return 0;

	case WM_MOUSEMOVE:
		{
			RECT rc = { 0 };
			this->getClientRect(&rc);
			int y = HIWORD(lp); // client area cursor coordinate

			if(this->_isDrag && (y > _Metrics::OFFSET && y < rc.bottom - _Metrics::OFFSET - _Metrics::GRIPWIDTH)) {
				this->_pos = y; // splitter grip position follows the cursor position
				this->sendMessage(WM_SIZE, SIZE_RESTORED, MAKELPARAM(rc.right, rc.bottom)); // will resize both panels
			} else if(y >= this->_pos && y <= this->_pos + _Metrics::GRIPWIDTH) {
				// Every time cursor is over the splitter grip, set the cursor.
				// While dragging, the cursor is already set by WM_LBUTTONDOWN,
				// so we don't need to set it again here.
				::SetCursor(::LoadCursor(0, IDC_SIZENS));
			}
		}
		return 0;
	}
	return Splitter::msgHandler(msg, wp, lp);
}