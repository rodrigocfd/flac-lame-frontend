//
// Part of TOOLOW - Thin Object Oriented Layer Over Win32.
//

#pragma once
#include "Window.h"
#include "Font.h"
#include <Commctrl.h>

class TextBox : public Window {
public:
	TextBox()                     { }
	TextBox(HWND hwnd)            { operator=(hwnd); }
	TextBox(const Window& wnd)    { operator=(wnd); }
	TextBox(const TextBox& other) { operator=(other); }

	TextBox&    operator=(HWND hwnd);
	TextBox&    operator=(const Window& wnd)    { return operator=(wnd.hWnd()); }
	TextBox&    operator=(const TextBox& other) { return operator=(other.hWnd()); }
	void        getTextLines(Array<String> *pBuf);
	TextBox&    setFont(const Font *pFont);
	const Font& getFont() const                 { return _font; }

private:
	static LRESULT CALLBACK _Proc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp, UINT_PTR idSubclass, DWORD_PTR refData);

	Font _font;
};