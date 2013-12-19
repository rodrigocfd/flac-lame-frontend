//
// Ordinary textbox automation.
// Part of TOOLOW - Thin Object Oriented Layer Over Win32.
//

#pragma once
#include "Window.h"
#include "Font.h"
#include <Commctrl.h>

class TextBox : public Window {
public:
	TextBox()                     : _notifyKeyUp(0) { }
	TextBox(HWND hwnd)            { operator=(hwnd); }
	TextBox(const Window& wnd)    { operator=(wnd); }
	TextBox(const TextBox& other) { operator=(other); }

	TextBox&      operator=(HWND hwnd);
	TextBox&      operator=(const Window& wnd)    { return operator=(wnd.hWnd()); }
	TextBox&      operator=(const TextBox& other) { return operator=(other.hWnd()); }
	int           getTextLen() const              { return ::GetWindowTextLength(hWnd()); }
	Array<String> getTextLines();
	TextBox&      setFont(const Font *pFont);
	const Font&   getFont() const                 { return _font; }
	TextBox&      selSetAll()                     { sendMessage(EM_SETSEL, 0, -1); return *this; }
	TextBox&      selSet(int start, int length)   { sendMessage(EM_SETSEL, start, start + length); return *this; }
	void          selGet(int *start, int *length);
	TextBox&      selReplace(const wchar_t *text) { sendMessage(EM_REPLACESEL, TRUE, (LPARAM)text); return *this; }
	TextBox&      notifyKeyUp(UINT msg)           { _notifyKeyUp = msg; return *this; }

private:
	static LRESULT CALLBACK _Proc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp, UINT_PTR idSubclass, DWORD_PTR refData);

	Font _font;
	UINT _notifyKeyUp;
};