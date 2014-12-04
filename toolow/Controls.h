//
// Often-used controls and related.
// Part of TOOLOW - Thin Object Oriented Layer Over Win32.
// @author Rodrigo Cesar de Freitas Dias
// @see https://github.com/rodrigocfd/toolow
//

#pragma once
#include "Window.h"
#include "Font.h"

//__________________________________________________________________________________________________
// Fit size and position of a bunch of controls inside a window.
//
class Resizer final {
public:
	enum class Do {
		REPOS,  // control size is fixed; control moves around anchored
		RESIZE, // control size stretches; control doesn't move
		NOTHING // control doesn't move or resize
	};
private:
	struct _Ctrl final {
		HWND hWnd;     // handle to child window
		RECT rcOrig;   // original coordinates relative to parent
		Do   modeHorz; // horizontal mode
		Do   modeVert; // vertical mode
	};

private:
	Array<_Ctrl> _ctrls;
	SIZE _szOrig;
	std::function<void()> _afterResize;
public:
	Resizer& create(int numCtrls);
	Resizer& add(initializer_list<HWND> hChildren, Do modeHorz, Do modeVert);
	Resizer& add(initializer_list<int> ctrlIds, HWND hParent, Do modeHorz, Do modeVert);
	Resizer& afterResize(std::function<void()> callback) { _afterResize = callback; return *this; }
private:
	void _addOne(HWND hCtrl, Do modeHorz, Do modeVert);
	static LRESULT CALLBACK _Proc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp, UINT_PTR idSubclass, DWORD_PTR refData);
};

//__________________________________________________________________________________________________
// Ordinary textbox with some utilities.
//
class TextBox : public Window {
private:
	Font _font;
	UINT _notifyKeyUp;
public:
	TextBox()                     : _notifyKeyUp(0) { }
	TextBox(HWND hwnd)            { operator=(hwnd); }
	TextBox(const Window& wnd)    { operator=(wnd); }
	TextBox(const TextBox& other) { operator=(other); }

	TextBox&      operator=(HWND hwnd);
	TextBox&      operator=(const Window& wnd)    { return operator=(wnd.hWnd()); }
	TextBox&      operator=(const TextBox& other) { return operator=(other.hWnd()); }
	int           getTextLen() const              { return ::GetWindowTextLength(hWnd()); }
	Array<String> getTextLines() const            { return getText().explode(L"\r\n"); }
	TextBox&      setFont(const Font& font);
	const Font&   getFont() const                 { return _font; }
	TextBox&      selSetAll()                     { sendMessage(EM_SETSEL, 0, -1); return *this; }
	TextBox&      selSet(int start, int length)   { sendMessage(EM_SETSEL, start, start + length); return *this; }
	void          selGet(int *start, int *length);
	TextBox&      selReplace(const wchar_t *text) { sendMessage(EM_REPLACESEL, TRUE, (LPARAM)text); return *this; }
	TextBox&      notifyKeyUp(UINT msg)           { _notifyKeyUp = msg; return *this; }

private:
	static LRESULT CALLBACK _Proc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp, UINT_PTR idSubclass, DWORD_PTR refData);
};

//__________________________________________________________________________________________________
// Regular combobox.
//
class Combo : public Window {
public:
	Combo()                   { }
	Combo(HWND hwnd)          { operator=(hwnd); }
	Combo(const Window& wnd)  { operator=(wnd); }
	Combo(const Combo& other) { operator=(other); }

	Combo&   operator=(HWND hwnd)          { ((Window*)this)->operator=(hwnd); return *this; }
	Combo&   operator=(const Window& wnd)  { return operator=(wnd.hWnd()); }
	Combo&   operator=(const Combo& other) { return operator=(other.hWnd()); }
	int      itemCount() const             { return (int)sendMessage(CB_GETCOUNT, 0, 0); }
	Combo&   itemSetSelected(int i)        { sendMessage(CB_SETCURSEL, i, 0); return *this; }
	int      itemGetSelected() const       { return (int)sendMessage(CB_GETCURSEL, 0, 0); }
	Combo&   itemAdd(initializer_list<const wchar_t*> arrStr);
	wchar_t* itemGetText(int i, wchar_t *pBuf, int szBuf) const;
	String*  itemGetText(int i, String *pBuf) const;
	String   itemGetText(int i) const      { String ret; itemGetText(i, &ret); return ret; }
	String   itemGetSelectedText() const   { return itemGetText(itemGetSelected()); }
	Combo&   itemRemoveAll()               { sendMessage(CB_RESETCONTENT, 0, 0); return *this; }
};

//__________________________________________________________________________________________________
// Regular listbox.
//
class ListBox : public Window {
public:
	ListBox()                     { }
	ListBox(HWND hwnd)            { operator=(hwnd); }
	ListBox(const Window& wnd)    { operator=(wnd); }
	ListBox(const ListBox& other) { operator=(other); }

	ListBox& operator=(HWND hwnd)            { ((Window*)this)->operator=(hwnd); return *this; }
	ListBox& operator=(const Window& wnd)    { return operator=(wnd.hWnd()); }
	ListBox& operator=(const ListBox& other) { return operator=(other.hWnd()); }
	ListBox& itemAdd(initializer_list<const wchar_t*> arrStr);
	int      itemCount() const               { return (int)sendMessage(LB_GETCOUNT, 0, 0); }
	int      itemCountSelected() const;
	int      itemGetSelected(Array<int> *indexesBuf=nullptr) const;
	wchar_t* itemGetText(int i, wchar_t *pBuf, int szBuf) const;
	String&  itemGetText(int i, String& buf) const;
	String   itemGetText(int i) const        { String ret; itemGetText(i, ret); return ret; }
	ListBox& itemRemoveAll()                 { sendMessage(LB_RESETCONTENT, 0, 0); return *this; }
};

//__________________________________________________________________________________________________
// Radio button.
//
class Radio : public Window {
public:
	enum class EmulateClick { EMULATE, NOEMULATE };

	Radio()                   { }
	Radio(HWND hwnd)          { operator=(hwnd); }
	Radio(const Window& wnd)  { operator=(wnd); }
	Radio(const Radio& other) { operator=(other); }

	Radio& operator=(HWND hwnd)          { ((Window*)this)->operator=(hwnd); return *this; }
	Radio& operator=(const Window& wnd)  { return operator=(wnd.hWnd()); }
	Radio& operator=(const Radio& other) { return operator=(other.hWnd()); }
	bool   isChecked()                   { return sendMessage(BM_GETCHECK, 0, 0) == BST_CHECKED; }
	void   setCheck(bool checked, EmulateClick emulateClick);
};

//__________________________________________________________________________________________________
// Checkbox; same behavior of a radio button.
//
class CheckBox : public Radio {
public:
	CheckBox()                      { }
	CheckBox(HWND hwnd)             { operator=(hwnd); }
	CheckBox(const Window& wnd)     { operator=(wnd); }
	CheckBox(const CheckBox& other) { operator=(other); }

	CheckBox& operator=(HWND hwnd)             { ((Window*)this)->operator=(hwnd); return *this; }
	CheckBox& operator=(const Window& wnd)     { return operator=(wnd.hWnd()); }
	CheckBox& operator=(const CheckBox& other) { return operator=(other.hWnd()); }
};

//__________________________________________________________________________________________________
// Progress bar.
//
class ProgressBar : public Window {
public:
	ProgressBar()                         { }
	ProgressBar(HWND hwnd)                { operator=(hwnd); }
	ProgressBar(const Window& wnd)        { operator=(wnd); }
	ProgressBar(const ProgressBar& other) { operator=(other); }

	ProgressBar& operator=(HWND hwnd)                { ((Window*)this)->operator=(hwnd); return *this; }
	ProgressBar& operator=(const Window& wnd)        { return operator=(wnd.hWnd()); }
	ProgressBar& operator=(const ProgressBar& other) { return operator=(other.hWnd()); }
	ProgressBar& setRange(int min, int max)          { sendMessage(PBM_SETRANGE, 0, MAKELPARAM(min, max)); return *this; }
	ProgressBar& setPos(int pos)                     { sendMessage(PBM_SETPOS, pos, 0); return *this; }
	ProgressBar& setPos(double pos)                  { return setPos(int(pos + 0.5)); }
	int          getPos()                            { return (int)sendMessage(PBM_GETPOS, 0, 0); }
	ProgressBar& animateMarquee(bool animate);
};

//__________________________________________________________________________________________________
// Status bar automation.
//
class StatusBar final {
private:
	struct _Part final { BYTE sizePixels; float resizeWeight; };

private:
	Window       _sb;
	Array<_Part> _parts;
	int          _lastInsertedPart;
	Array<int>   _rightEdges;
public:
	StatusBar& create(HWND hOwner, int numPartsItWillHave);
	StatusBar& addFixedPart(BYTE sizePixels);
	StatusBar& addResizablePart(float resizeWeight);
	StatusBar& setText(const wchar_t *text, int iPart) { _sb.sendMessage(SB_SETTEXT, MAKEWPARAM(MAKEWORD(iPart, 0), 0), (LPARAM)text); return *this; }
	String     getText(int iPart) const;
	void       setIcon(HICON hIcon, int iPart)         { _sb.sendMessage(SB_SETICON, iPart, (LPARAM)hIcon); }
	void       doResize(WPARAM wp, LPARAM lp)          { if (wp != SIZE_MINIMIZED && _sb.hWnd()) _putParts(LOWORD(lp)); }
private:
	void _putParts(int cx);
};

//__________________________________________________________________________________________________
// HICON automation.
//
class Icon final {
private:
	HICON _hIcon;
public:
	Icon()  : _hIcon(nullptr) { }
	~Icon() { this->free(); }

	HICON hIcon() const          { return _hIcon; }
	Icon& free()                 { if (_hIcon) ::DestroyIcon(_hIcon); return *this; }
	Icon& operator=(HICON hIcon) { _hIcon = hIcon; return *this; }
	Icon& getFromExplorer(const wchar_t *fileExtension);
	Icon& getFromResource(int iconId, int size, HINSTANCE hInst=nullptr);

	static void IconToLabel(HWND hStatic, int idIconRes, BYTE size);
};