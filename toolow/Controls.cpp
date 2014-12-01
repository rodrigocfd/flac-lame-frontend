//
// Often-used controls and related.
// Part of TOOLOW - Thin Object Oriented Layer Over Win32.
// @author Rodrigo Cesar de Freitas Dias
// @see https://github.com/rodrigocfd/toolow
//

#include "Controls.h"
#include "System.h"

Resizer& Resizer::create(int numCtrls)
{
	_ctrls.reserve(numCtrls); // just to save reallocs
	return *this;
}

Resizer& Resizer::add(initializer_list<HWND> hChildren, Do modeHorz, Do modeVert)
{
	for(int i = 0; i < (int)hChildren.size(); ++i)
		this->_addOne(*(hChildren.begin() + i), modeHorz, modeVert);
	return *this;
}

Resizer& Resizer::add(initializer_list<int> ctrlIds, HWND hParent, Do modeHorz, Do modeVert)
{
	for(int i = 0; i < (int)ctrlIds.size(); ++i)
		this->_addOne(GetDlgItem(hParent, *(ctrlIds.begin() + i)), modeHorz, modeVert);
	return *this;
}

void Resizer::_addOne(HWND hCtrl, Do modeHorz, Do modeVert)
{
	if(!_ctrls.size()) { // first call to _addOne()
		RECT rcP;
		GetClientRect(GetParent(hCtrl), &rcP);
		_szOrig.cx = rcP.right;
		_szOrig.cy = rcP.bottom; // save original size of parent
		SetWindowSubclass(GetParent(hCtrl), _Proc, 1, (DWORD_PTR)this); // subclass parent, we'll manage WM_SIZE
	}

	_ctrls.append(_Ctrl());
	_ctrls.last().hWnd = hCtrl;
	_ctrls.last().modeHorz = modeHorz;
	_ctrls.last().modeVert = modeVert;

	GetWindowRect(_ctrls.last().hWnd, &_ctrls.last().rcOrig);
	ScreenToClient(GetParent(hCtrl), (POINT*)&_ctrls.last().rcOrig);
	ScreenToClient(GetParent(hCtrl), (POINT*)&_ctrls.last().rcOrig.right); // client coordinates relative to parent
}

LRESULT CALLBACK Resizer::_Proc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp, UINT_PTR idSubclass, DWORD_PTR refData)
{
	switch(msg)
	{
	case WM_SIZE:
		{
			Resizer *pThis = (Resizer*)refData;
			int state = (int)wp;
			int cx = LOWORD(lp);
			int cy = HIWORD(lp);
			if(pThis->_ctrls.size() && state != SIZE_MINIMIZED) { // only if created() was called; if minimized, no need to resize
				HDWP hdwp = BeginDeferWindowPos(pThis->_ctrls.size());
				for(int i = 0; i < pThis->_ctrls.size(); ++i) {
					_Ctrl *pCtrl = &pThis->_ctrls[i]; // current child control being worked with

					UINT uFlags = SWP_NOZORDER;
					if(pCtrl->modeHorz == Do::REPOS && pCtrl->modeVert == Do::REPOS) // reposition both vert & horz
						uFlags |= SWP_NOSIZE;
					else if(pCtrl->modeHorz == Do::RESIZE && pCtrl->modeVert == Do::RESIZE) // resize both vert & horz
						uFlags |= SWP_NOMOVE;

					DeferWindowPos(hdwp, pCtrl->hWnd, nullptr,
						pCtrl->modeHorz == Do::REPOS ?
							cx - pThis->_szOrig.cx + pCtrl->rcOrig.left :
							pCtrl->rcOrig.left, // keep original pos
						pCtrl->modeVert == Do::REPOS ?
							cy - pThis->_szOrig.cy + pCtrl->rcOrig.top :
							pCtrl->rcOrig.top, // keep original pos
						pCtrl->modeHorz == Do::RESIZE ?
							cx - pThis->_szOrig.cx + pCtrl->rcOrig.right - pCtrl->rcOrig.left :
							pCtrl->rcOrig.right - pCtrl->rcOrig.left, // keep original width
						pCtrl->modeVert == Do::RESIZE ?
							cy - pThis->_szOrig.cy + pCtrl->rcOrig.bottom - pCtrl->rcOrig.top :
							pCtrl->rcOrig.bottom - pCtrl->rcOrig.top, // keep original height
						uFlags);
				}
				EndDeferWindowPos(hdwp);
				if(pThis->_afterResize) pThis->_afterResize(); // invoke user callback, if any
			}
		}
		break;
	case WM_NCDESTROY:
		RemoveWindowSubclass(hWnd, _Proc, idSubclass); // http://blogs.msdn.com/b/oldnewthing/archive/2003/11/11/55653.aspx
	}
	return DefSubclassProc(hWnd, msg, wp, lp);
}


TextBox& TextBox::operator=(HWND hwnd)
{
	const int IDSUBCLASS = 1;

	if(this->hWnd()) // remove any previous subclassing of us, will be reassigned
		RemoveWindowSubclass(this->hWnd(), _Proc, IDSUBCLASS);

	((Window*)this)->operator=(hwnd);
	_notifyKeyUp = 0;
	SetWindowSubclass(this->hWnd(), _Proc, IDSUBCLASS, (DWORD_PTR)this);
	return *this;
}

TextBox& TextBox::setFont(const Font& font)
{
	// Call this method within WM_SHOWWINDOW processing.
	// If called during WM_INITDIALOG, it will be undone during default processing.

	_font.cloneFrom(font); // since it's cloned, user font may be safely deleted
	_font.apply(hWnd());
	return *this;
}

void TextBox::selGet(int *start, int *length)
{
	int p1 = 0;
	this->sendMessage(EM_GETSEL, (WPARAM)start, (LPARAM)&p1);
	*length = p1 - (*start);
}

LRESULT CALLBACK TextBox::_Proc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp, UINT_PTR idSubclass, DWORD_PTR refData)
{
	TextBox *pSelf = (TextBox*)refData;

	switch(msg)
	{
	case WM_KEYDOWN:
		// http://www.williamwilling.com/blog/?p=28
		switch(LOWORD(wp))
		{
		case VK_ESCAPE: // ESC
			SendMessage(GetAncestor(hWnd, GA_PARENT), WM_COMMAND, IDCANCEL, (LPARAM)hWnd);
			return 0;
		}
		break;
	case WM_GETDLGCODE:
		if(lp && wp == 'A' && System::HasCtrl()) { // Ctrl+A to select all text
			((MSG*)lp)->wParam = 0; // prevent propagation, therefore beep
			SendMessage(hWnd, EM_SETSEL, 0, -1);
			return DLGC_WANTCHARS;
		}
		break;
	case WM_KEYUP:
		if(pSelf->_notifyKeyUp) pSelf->getParent().sendMessage(pSelf->_notifyKeyUp, wp, lp);
		break;
	case WM_NCDESTROY:
		RemoveWindowSubclass(hWnd, _Proc, idSubclass);
	}
	return DefSubclassProc(hWnd, msg, wp, lp);
}


Combo& Combo::itemAdd(initializer_list<const wchar_t*> arrStr)
{
	for(int i = 0; i < (int)arrStr.size(); ++i)
		this->sendMessage(CB_ADDSTRING, 0, (LPARAM)*(arrStr.begin() + i));
	return *this;
}

wchar_t* Combo::itemGetText(int i, wchar_t *pBuf, int szBuf) const
{
	int len = (int)this->sendMessage(CB_GETLBTEXTLEN, i, 0) + 1;
	if(szBuf < len) *pBuf = L'\0'; // buffer is too small
	else this->sendMessage(CB_GETLBTEXT, i, (LPARAM)pBuf);
	return pBuf;
}

String* Combo::itemGetText(int i, String *pBuf) const
{
	pBuf->reserve((int)this->sendMessage(CB_GETLBTEXTLEN, i, 0));
	this->sendMessage(CB_GETLBTEXT, i, (LPARAM)pBuf->ptrAt(0));
	return pBuf;
}


ListBox& ListBox::itemAdd(initializer_list<const wchar_t*> arrStr)
{
	for(int i = 0; i < (int)arrStr.size(); ++i)
		this->sendMessage(LB_ADDSTRING, 0, (LPARAM)*(arrStr.begin() + i));
	return *this;
}

int ListBox::itemCountSelected() const
{
	int cou = (int)this->sendMessage(LB_GETSELCOUNT, 0, 0);
	if(cou == LB_ERR) // we have a single-selection listbox, zero or one items can be selected
		return this->sendMessage(LB_GETCURSEL, 0, 0) == LB_ERR ? 0 : 1;
	return cou;
}

int ListBox::itemGetSelected(Array<int> *indexesBuf) const
{
	if(indexesBuf) {
		indexesBuf->resize(itemCountSelected());
		if(this->sendMessage(LB_GETSELITEMS, (WPARAM)indexesBuf->size(), (LPARAM)&(*indexesBuf)[0]) == LB_ERR)
			if(indexesBuf->size() > 0) // a single-selection listbox
				(*indexesBuf)[0] = (int)this->sendMessage(LB_GETCURSEL, 0, 0);
		return indexesBuf->size() > 0 ? (*indexesBuf)[0] : -1;
	}
	return (int)this->sendMessage(LB_GETCURSEL, 0, 0); // will work for single-selection listbox only
}

wchar_t* ListBox::itemGetText(int i, wchar_t *pBuf, int szBuf) const
{
	int len = (int)this->sendMessage(LB_GETTEXTLEN, i, 0) + 1;
	if(szBuf < len) *pBuf = 0; // buffer is too small
	else this->sendMessage(LB_GETTEXT, i, (LPARAM)pBuf);
	return pBuf;
}

String& ListBox::itemGetText(int i, String& buf) const
{
	buf.reserve((int)this->sendMessage(LB_GETTEXTLEN, i, 0));
	this->sendMessage(LB_GETTEXT, i, (LPARAM)buf.ptrAt(0));
	return buf;
}


void Radio::setCheck(bool checked, Radio::EmulateClick emulateClick)
{
	this->sendMessage(BM_SETCHECK, checked ? BST_CHECKED : BST_UNCHECKED, 0);
	if(emulateClick == EmulateClick::EMULATE)
		this->getParent().sendMessage(WM_COMMAND, MAKEWPARAM(GetDlgCtrlID(this->hWnd()), 0), (LPARAM)this->hWnd());
}


ProgressBar& ProgressBar::animateMarquee(bool animate)
{
	SetWindowLongPtr(this->hWnd(), GWL_STYLE,
		GetWindowLongPtr(this->hWnd(), GWL_STYLE) | PBS_MARQUEE); // set this on resource editor won't work
	this->sendMessage(PBM_SETMARQUEE, (WPARAM)animate, 0);
	return *this;
}


StatusBar& StatusBar::create(HWND hOwner, int numPartsItWillHave)
{
	// The owner is considered resizable if it has the maximize button.
	bool isStretch = (GetWindowLongPtr(hOwner, GWL_STYLE) & WS_MAXIMIZEBOX) != 0;

	_sb = CreateWindowEx(0, STATUSCLASSNAME, nullptr,
		(isStretch ? SBARS_SIZEGRIP : 0) | WS_CHILD | WS_VISIBLE,
		0, 0, 0, 0, hOwner, nullptr, (HINSTANCE)GetWindowLongPtr(hOwner, GWLP_HINSTANCE), 0);

	_parts.resize(numPartsItWillHave);
	_lastInsertedPart = -1;
	_rightEdges.resize(numPartsItWillHave); // only used during resizing; allocated through object life for performance
	return *this;
}

StatusBar& StatusBar::addFixedPart(BYTE sizePixels)
{
	++_lastInsertedPart;
	if(_lastInsertedPart >= _parts.size()) // buffer overrun protection
		return *this;

	_parts[_lastInsertedPart].sizePixels = sizePixels;
	_parts[_lastInsertedPart].resizeWeight = 0;

	if(_lastInsertedPart == _parts.size() - 1) {
		RECT rc = _sb.getParent().getClientRect();
		this->_putParts(rc.right);
	}
	return *this;
}

StatusBar& StatusBar::addResizablePart(float resizeWeight)
{
	// How resizeWeight works:
	// Suppose you have 3 parts, respectively with weights of 1, 1 and 2.
	// If available client area is 400px, respective part widths will be 100, 100 and 200px.
	// Zero weight means a fixed-width part, which therefore should have sizePixels set (otherwise zero).

	++_lastInsertedPart;
	if(_lastInsertedPart >= _parts.size()) // buffer overrun protection
		return *this;

	_parts[_lastInsertedPart].sizePixels = 0;
	_parts[_lastInsertedPart].resizeWeight = resizeWeight;

	if(_lastInsertedPart == _parts.size() - 1) {
		RECT rc = _sb.getParent().getClientRect();
		this->_putParts(rc.right);
	}
	return *this;
}

String StatusBar::getText(int iPart) const
{
	int len = LOWORD(_sb.sendMessage(SB_GETTEXTLENGTH, iPart, 0));
	String ret;
	ret.reserve(len);
	_sb.sendMessage(SB_GETTEXT, iPart, (LPARAM)ret.ptrAt(0));
	return ret;
}

void StatusBar::_putParts(int cx)
{
	_sb.sendMessage(WM_SIZE, 0, 0); // tell statusbar to fit parent

	// Find the space to be divided among variable-width parts,
	// and total weight of variable-width parts.
	float totalWeight = 0;
	int   cxVariable = cx, cxTotal = cx;

	for(int i = 0; i < _parts.size(); ++i) {
		if(!_parts[i].resizeWeight) // fixed-width?
			cxVariable -= _parts[i].sizePixels;
		else
			totalWeight += _parts[i].resizeWeight;
	}

	// Fill right edges array with the right edge of each part.
	for(int i = _parts.size() - 1; i >= 0; --i) {
		_rightEdges[i] = cxTotal;
		cxTotal -= (!_parts[i].resizeWeight) ? // fixed-width?
			_parts[i].sizePixels :
			(int)( (cxVariable / totalWeight) * _parts[i].resizeWeight );
	}

	_sb.sendMessage(SB_SETPARTS, _rightEdges.size(), (LPARAM)&_rightEdges[0]);
}


Icon& Icon::getFromExplorer(const wchar_t *fileExtension)
{
	this->free();
	wchar_t extens[10];
	lstrcpy(extens, L"*.");
	lstrcat(extens, fileExtension); // prefix extension; caller must pass just the 3 letters (or 4, whathever)
	SHFILEINFO shfi = { 0 };
	SHGetFileInfo(extens, FILE_ATTRIBUTE_NORMAL, &shfi, sizeof(shfi),
		SHGFI_TYPENAME | SHGFI_USEFILEATTRIBUTES);
	SHGetFileInfo(extens, FILE_ATTRIBUTE_NORMAL, &shfi, sizeof(shfi),
		SHGFI_ICON | SHGFI_SMALLICON | SHGFI_SYSICONINDEX | SHGFI_USEFILEATTRIBUTES);
	_hIcon = shfi.hIcon;
	return *this;
}

Icon& Icon::getFromResource(int iconId, int size, HINSTANCE hInst)
{
	// The size should be 16, 32, 48 or any other size the icon eventually has.
	_hIcon = (HICON)LoadImage(hInst ? hInst : GetModuleHandle(nullptr),
		MAKEINTRESOURCE(iconId), IMAGE_ICON, size, size, LR_DEFAULTCOLOR);
	return *this;
}

void Icon::IconToLabel(HWND hStatic, int idIconRes, BYTE size)
{
	// Loads an icon resource into a static control placed on a dialog.
	// On the resource editor, change "Type" property to "Icon".
	// The size should be 16, 32, 48 or any other size the icon eventually has.
	SendMessage(hStatic, STM_SETIMAGE, IMAGE_ICON,
		(LPARAM)(HICON)LoadImage(
			(HINSTANCE)GetWindowLongPtr(GetParent(hStatic), GWLP_HINSTANCE),
			MAKEINTRESOURCE(idIconRes), IMAGE_ICON, size, size, LR_DEFAULTCOLOR) );
}