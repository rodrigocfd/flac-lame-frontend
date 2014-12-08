//
// Often-used controls and related.
// Part of WOLF - Win32 Object Lambda Framework.
// @author Rodrigo Cesar de Freitas Dias
// @see https://github.com/rodrigocfd/wolf
//

#pragma once
#include "Resources.h"
#include "Window.h"

// Fit size and position of a bunch of controls inside a window.
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


// Ordinary textbox with some utilities.
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


// Regular combobox.
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


// Regular listbox.
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


// Radio button.
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


// Checkbox; same behavior of a radio button.
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


// Progress bar.
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


// Status bar automation.
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


// The complex listview.
class ListView final : public Window {
public:
	class Item final {
	private:
		ListView *_list;
	public:
		int i;
		Item()                           : i(-1), _list(nullptr) { }
		Item(int index, ListView *pList) : i(index), _list(pList) { }
		void     remove()                { ListView_DeleteItem(_list->hWnd(), i); }
		void     swapWith(int index);
		Item&    ensureVisible();
		bool     isVisible() const       { return ListView_IsItemVisible(_list->hWnd(), i) == TRUE; }
		Item&    setSelect(bool select)  { ListView_SetItemState(_list->hWnd(), i, select ? LVIS_SELECTED : 0, LVIS_SELECTED); return *this; }
		bool     isSelected() const      { return (ListView_GetItemState(_list->hWnd(), i, LVIS_SELECTED) & LVIS_SELECTED) != 0; }
		Item&    setFocus()              { ListView_SetItemState(_list->hWnd(), i, LVIS_FOCUSED, LVIS_FOCUSED); return *this; }
		bool     isFocused() const       { return (ListView_GetItemState(_list->hWnd(), i, LVIS_FOCUSED) & LVIS_FOCUSED) != 0; }
		RECT     getRect() const         { RECT r = { 0 }; ListView_GetItemRect(_list->hWnd(), i, &r, LVIR_BOUNDS); return r; }
		wchar_t* getText(wchar_t *pBuf, int szBuf, int iCol=0) const { ListView_GetItemText(_list->hWnd(), i, iCol, pBuf, szBuf); return pBuf; }
		String&  getText(String& buf, int iCol=0) const;
		String   getText(int iCol=0) const                           { String ret; getText(ret, iCol); return ret; }
		Item&    setText(const wchar_t *text, int iCol=0)            { ListView_SetItemText(_list->hWnd(), i, iCol, (wchar_t*)text); return *this; }
		Item&    setText(const String& text, int iCol=0)             { return setText(text.str(), iCol); }
		LPARAM   getParam() const;
		Item&    setParam(LPARAM lp);
		int      getIcon() const;
		Item&    setIcon(int iconIdx);
	};

	class ItemsProxy final {
	private:
		ListView *_list;
	public:
		explicit ItemsProxy(ListView *pList) : _list(pList) { }
		Item        operator[](int index)          { return Item(index, _list); }
		int         count() const                  { return ListView_GetItemCount(_list->hWnd()); }
		Item        add(const wchar_t *caption, int iconIdx=-1, int at=-1);
		Item        add(const String& caption, int iconIdx=-1, int at=-1) { return add(caption.str(), iconIdx, at); }
		Array<Item> getAll() const;
		void        removeAll()                    { ListView_DeleteAllItems(_list->hWnd()); }
		Item        find(const wchar_t *caption);
		Item        find(const String& caption)    { return find(caption.str()); }
		bool        exists(const wchar_t *caption) { return find(caption).i != -1; }
		bool        exists(const String& caption)  { return exists(caption.str()); }
		int         countSelected() const          { return ListView_GetSelectedCount(_list->hWnd()); }
		void        select(const Array<int>& idx);
		void        selectAll()                    { ListView_SetItemState(_list->hWnd(), -1, LVIS_SELECTED, LVIS_SELECTED); }
		void        selectNone()                   { ListView_SetItemState(_list->hWnd(), -1, 0, LVIS_SELECTED); }
		void        removeSelected();
		Array<Item> getSelected() const;
		Item        getFocused() const             { return Item(ListView_GetNextItem(_list->hWnd(), -1, LVNI_FOCUSED), _list); }
	};
private:
	int _ctxMenuId;
public:
	ItemsProxy items;
	enum class View { VW_DETAILS=LV_VIEW_DETAILS, VW_ICON=LV_VIEW_ICON, VW_LIST=LV_VIEW_LIST, VW_SMALLICON=LV_VIEW_SMALLICON, VW_TILE=LV_VIEW_TILE };

	ListView()                      : items(nullptr), _ctxMenuId(0) { }
	ListView(HWND hwnd)             : items(nullptr), _ctxMenuId(0) { operator=(hwnd); }
	ListView(const Window& wnd)     : items(nullptr), _ctxMenuId(0) { operator=(wnd); }
	ListView(const ListView& other) : items(nullptr), _ctxMenuId(0) { operator=(other); }

	ListView& operator=(HWND hwnd);
	ListView& operator=(const Window& wnd)     { return operator=(wnd.hWnd()); }
	ListView& operator=(const ListView& other) { operator=(other.hWnd()); _ctxMenuId = other._ctxMenuId; return *this; }

	ListView& setFullRowSelect()         { ListView_SetExtendedListViewStyle(hWnd(), LVS_EX_FULLROWSELECT); return *this; }
	ListView& setRedraw(bool doRedraw)   { sendMessage(WM_SETREDRAW, (WPARAM)(BOOL)doRedraw, 0); return *this; }
	ListView& setContextMenu(int menuId) { _ctxMenuId = menuId; return *this; }
	ListView& setView(View view)         { ListView_SetView(hWnd(), (DWORD)view); return *this; }
	View      getView() const            { return (View)ListView_GetView(hWnd()); }
	
	ListView& iconPush(int iconId);
	ListView& iconPush(const wchar_t *fileExtension);

	int       columnCount() const        { return Header_GetItemCount(ListView_GetHeader(hWnd())); }
	ListView& columnAdd(const wchar_t *caption, int cx);
	ListView& columnFit(int iCol);
private:
	HIMAGELIST _proceedImageList();
	int        _showCtxMenu(bool followCursor);
	static LRESULT CALLBACK _Proc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp, UINT_PTR idSubclass, DWORD_PTR refData);
};