
#include "TextBox.h"
#include "util.h"
#pragma comment(lib, "ComCtl32.lib")

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

Array<String> TextBox::getTextLines()
{
	String text;
	this->getText(&text);
	return text.explode(L"\r\n");
}

TextBox& TextBox::setFont(const Font *pFont)
{
	// Call this method within WM_SHOWWINDOW processing.
	// If called during WM_INITDIALOG, it will be undone during default processing.

	_font.cloneFrom(pFont); // since it's cloned, user font may be safely deleted
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
		if(lp && wp == 'A' && hasCtrl()) { // Ctrl+A to select all text
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