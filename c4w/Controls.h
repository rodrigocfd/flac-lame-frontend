/*!
 * Often-used controls.
 * Part of C4W - Classes for Win32.
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/c4w
 */

#pragma once
#include "Resources.h"
#include "Str.h"
#include "Window.h"

namespace c4w {

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
		Window wnd;      // handle to child window
		RECT   rcOrig;   // original coordinates relative to parent
		Do     modeHorz; // horizontal mode
		Do     modeVert; // vertical mode
	};

private:
	std::vector<_Ctrl> _ctrls;
	SIZE _szOrig;
	std::function<void()> _afterResize;
public:
	Resizer& add(std::initializer_list<HWND> hChildren, Do modeHorz, Do modeVert);
	Resizer& add(std::initializer_list<int> ctrlIds, HWND hParent, Do modeHorz, Do modeVert);
	Resizer& afterResize(std::function<void()> callback) { _afterResize = std::move(callback); return *this; }
private:
	void _addOne(Window ctrl, Do modeHorz, Do modeVert);
	static LRESULT CALLBACK _Proc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp, UINT_PTR idSubclass, DWORD_PTR refData);
};


// HMENU wrapper.
class Menu {
private:
	HMENU _hMenu;
public:
	Menu()            : _hMenu(nullptr) { }
	Menu(HMENU hMenu) : _hMenu(hMenu) { }
	
	HMENU hMenu() const             { return _hMenu; }
	int   size() const              { return ::GetMenuItemCount(_hMenu); }
	void  destroy()                 { if (_hMenu) { ::DestroyMenu(_hMenu); _hMenu = nullptr; } }
	Menu  getSubmenu(int pos) const { return Menu(::GetSubMenu(_hMenu, pos)); }
	WORD  getCmdId(int pos) const   { return ::GetMenuItemID(_hMenu, pos); }
	Menu& createMain(HWND owner);
	Menu& createPopup();
	Menu& appendSeparator();
	Menu& appendItem(const wchar_t *caption, WORD cmdId);
	Menu& appendItem(const std::wstring& caption, WORD cmdId) { return appendItem(caption.c_str(), cmdId); }
	Menu& enableItem(std::initializer_list<WORD> cmdIds, bool doEnable);
	Menu  appendSubmenu(const wchar_t *caption);
	Menu  appendSubmenu(const std::wstring& caption)          { return appendSubmenu(caption.c_str()); }
private:
	void _checkDummyEntry();
};


// Ordinary textbox with some utilities.
class TextBox : public Window {
private:
	Font _font;
	UINT _notifyKeyUp;
	std::function<void(WORD)> _onKeyUp;
public:
	TextBox()                     : _notifyKeyUp(0) { }
	TextBox(HWND hwnd)            { operator=(hwnd); }
	TextBox(const Window& wnd)    { operator=(wnd); }
	TextBox(const TextBox& other) { operator=(other); }

	TextBox&    operator=(HWND hwnd);
	TextBox&    operator=(const Window& wnd)                { return operator=(wnd.hWnd()); }
	TextBox&    operator=(const TextBox& other)             { return operator=(other.hWnd()); }
	TextBox&    create(WindowPopup *parent, int id, POINT pos, int cx, UINT extraStyles=0);
	int         getTextLen() const                          { return ::GetWindowTextLength(hWnd()); }
	std::vector<std::wstring> getTextLines() const          { return str::Explode(getText(), L"\r\n"); }
	TextBox&    setFont(const Font& font);
	const Font& getFont() const                             { return _font; }
	TextBox&    selSetAll()                                 { sendMessage(EM_SETSEL, 0, -1); return *this; }
	TextBox&    selSet(int start, int length)               { sendMessage(EM_SETSEL, start, start + length); return *this; }
	void        selGet(int *start, int *length);
	TextBox&    selReplace(const wchar_t *text)             { sendMessage(EM_REPLACESEL, TRUE, reinterpret_cast<LPARAM>(text)); return *this; }
	TextBox&    selReplace(const std::wstring text)         { return selReplace(text.c_str()); }
	void        onKeyUp(std::function<void(WORD)> callback) { _onKeyUp = callback; }
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

	Combo&   operator=(HWND hwnd)            { static_cast<Window*>(this)->operator=(hwnd); return *this; }
	Combo&   operator=(const Window& wnd)    { return operator=(wnd.hWnd()); }
	Combo&   operator=(const Combo& other)   { return operator=(other.hWnd()); }
	Combo&   create(WindowPopup *parent, int id, POINT pos, int cx);
	int      itemCount() const               { return static_cast<int>(sendMessage(CB_GETCOUNT, 0, 0)); }
	Combo&   itemRemoveAll()                 { sendMessage(CB_RESETCONTENT, 0, 0); return *this; }
	Combo&   itemSetSelected(int i)          { sendMessage(CB_SETCURSEL, i, 0); return *this; }
	int      itemGetSelected() const         { return static_cast<int>(sendMessage(CB_GETCURSEL, 0, 0)); }
	Combo&   itemAdd(std::initializer_list<const wchar_t*> arrStr);
	wchar_t*     itemGetText(int i, wchar_t *pBuf, int szBuf) const;
	std::wstring itemGetText(int i) const;
	std::wstring itemGetSelectedText() const { return itemGetText(itemGetSelected()); }
};


// Regular listbox.
class ListBox : public Window {
public:
	ListBox()                     { }
	ListBox(HWND hwnd)            { operator=(hwnd); }
	ListBox(const Window& wnd)    { operator=(wnd); }
	ListBox(const ListBox& other) { operator=(other); }

	ListBox& operator=(HWND hwnd)            { static_cast<Window*>(this)->operator=(hwnd); return *this; }
	ListBox& operator=(const Window& wnd)    { return operator=(wnd.hWnd()); }
	ListBox& operator=(const ListBox& other) { return operator=(other.hWnd()); }
	ListBox& create(WindowPopup *parent, int id, POINT pos, SIZE size);
	ListBox& itemAdd(std::initializer_list<const wchar_t*> arrStr);
	int      itemCount() const               { return static_cast<int>(sendMessage(LB_GETCOUNT, 0, 0)); }
	int      itemCountSelected() const;
	int      itemGetSelected(std::vector<int> *indexesBuf=nullptr) const;
	wchar_t*     itemGetText(int i, wchar_t *pBuf, int szBuf) const;
	std::wstring itemGetText(int i) const;
	ListBox& itemRemoveAll()                 { sendMessage(LB_RESETCONTENT, 0, 0); return *this; }
};


// Radio button.
class Radio : public Window {
public:
	enum class EmulateClick { YES, NO };

	Radio()                   { }
	Radio(HWND hwnd)          { operator=(hwnd); }
	Radio(const Window& wnd)  { operator=(wnd); }
	Radio(const Radio& other) { operator=(other); }

	Radio& operator=(HWND hwnd)          { static_cast<Window*>(this)->operator=(hwnd); return *this; }
	Radio& operator=(const Window& wnd)  { return operator=(wnd.hWnd()); }
	Radio& operator=(const Radio& other) { return operator=(other.hWnd()); }
	Radio& create(WindowPopup *parent, int id, const wchar_t *caption, bool beginGroup, POINT pos, SIZE size);
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

	CheckBox& operator=(HWND hwnd)             { static_cast<Window*>(this)->operator=(hwnd); return *this; }
	CheckBox& operator=(const Window& wnd)     { return operator=(wnd.hWnd()); }
	CheckBox& operator=(const CheckBox& other) { return operator=(other.hWnd()); }
	CheckBox& create(WindowPopup *parent, int id, const wchar_t *caption, POINT pos, SIZE size);
};


// Progress bar.
class ProgressBar final : public Window {
public:
	ProgressBar()                         { }
	ProgressBar(HWND hwnd)                { operator=(hwnd); }
	ProgressBar(const Window& wnd)        { operator=(wnd); }
	ProgressBar(const ProgressBar& other) { operator=(other); }

	ProgressBar& operator=(HWND hwnd)                   { static_cast<Window*>(this)->operator=(hwnd); return *this; }
	ProgressBar& operator=(const Window& wnd)           { return operator=(wnd.hWnd()); }
	ProgressBar& operator=(const ProgressBar& other)    { return operator=(other.hWnd()); }
	ProgressBar& create(WindowPopup *parent, int id, POINT pos, SIZE size);
	ProgressBar& setRange(int minVal, int maxVal)       { sendMessage(PBM_SETRANGE, 0, MAKELPARAM(minVal, maxVal)); return *this; }
	ProgressBar& setRange(int minVal, size_t maxVal)    { return setRange(minVal, static_cast<int>(maxVal)); }
	ProgressBar& setPos(int pos)                        { sendMessage(PBM_SETPOS, pos, 0); return *this; }
	ProgressBar& setPos(size_t pos)                     { return setPos(static_cast<int>(pos)); }
	ProgressBar& setPos(double pos)                     { return setPos(static_cast<int>(pos + 0.5)); }
	int          getPos()                               { return static_cast<int>(sendMessage(PBM_GETPOS, 0, 0)); }
	ProgressBar& animateMarquee(bool animate);
};


// Status bar automation.
class StatusBar final {
private:
	struct _Part final { BYTE sizePixels; float resizeWeight; };
private:
	Window             _sb;
	std::vector<_Part> _parts;
	std::vector<int>   _rightEdges;
public:
	StatusBar& create(HWND hOwner, int numPartsItWillHave);
	StatusBar& addFixedPart(BYTE sizePixels);
	StatusBar& addResizablePart(float resizeWeight);
	StatusBar& setText(const wchar_t *text, int iPart);
	StatusBar& setText(const std::wstring& text, int iPart) { return setText(text.c_str(), iPart); }
	wchar_t*     getText(int iPart, wchar_t *pBuf, int szBuf) const;
	std::wstring getText(int iPart) const;
	void setIcon(HICON hIcon, int iPart) { _sb.sendMessage(SB_SETICON, iPart, reinterpret_cast<LPARAM>(hIcon)); }
	void doResize(WPARAM wp, LPARAM lp)  { if (wp != SIZE_MINIMIZED && _sb.hWnd()) _putParts(LOWORD(lp)); }
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
		Item(int index, ListView *pList) : i(index), _list(pList) { }
		Item()                           : Item(-1, nullptr) { }
		void     remove()                { ListView_DeleteItem(_list->hWnd(), i); }
		void     swapWith(int index);
		Item&    ensureVisible();
		bool     isVisible() const       { return ListView_IsItemVisible(_list->hWnd(), i) == TRUE; }
		Item&    setSelect(bool select)  { ListView_SetItemState(_list->hWnd(), i, select ? LVIS_SELECTED : 0, LVIS_SELECTED); return *this; }
		bool     isSelected() const      { return (ListView_GetItemState(_list->hWnd(), i, LVIS_SELECTED) & LVIS_SELECTED) != 0; }
		Item&    setFocus()              { ListView_SetItemState(_list->hWnd(), i, LVIS_FOCUSED, LVIS_FOCUSED); return *this; }
		bool     isFocused() const       { return (ListView_GetItemState(_list->hWnd(), i, LVIS_FOCUSED) & LVIS_FOCUSED) != 0; }
		RECT     getRect() const         { RECT r = { 0 }; ListView_GetItemRect(_list->hWnd(), i, &r, LVIR_BOUNDS); return r; }
		wchar_t*     getText(wchar_t *pBuf, int szBuf, int iCol=0) const { ListView_GetItemText(_list->hWnd(), i, iCol, pBuf, szBuf); return pBuf; }
		std::wstring getText(int iCol=0) const;
		Item&    setText(const wchar_t *text, int iCol=0)      { ListView_SetItemText(_list->hWnd(), i, iCol, const_cast<wchar_t*>(text)); return *this; }
		Item&    setText(const std::wstring& text, int iCol=0) { return setText(text.c_str(), iCol); }
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
		Item operator[](int index)               { return Item(index, _list); }
		int  count() const                       { return ListView_GetItemCount(_list->hWnd()); }
		Item add(const wchar_t *caption, int iconIdx=-1, int at=-1);
		Item add(const std::wstring& caption, int iconIdx=-1, int at=-1) { return add(caption.c_str(), iconIdx, at); }
		std::vector<Item> getAll() const;
		void removeAll()                         { ListView_DeleteAllItems(_list->hWnd()); }
		Item find(const wchar_t *caption);
		Item find(const std::wstring& caption)   { return find(caption.c_str()); }
		bool exists(const wchar_t *caption)      { return find(caption).i != -1; }
		bool exists(const std::wstring& caption) { return exists(caption.c_str()); }
		int  countSelected() const               { return ListView_GetSelectedCount(_list->hWnd()); }
		void select(const std::vector<int>& idx);
		void selectAll()                         { ListView_SetItemState(_list->hWnd(), -1, LVIS_SELECTED, LVIS_SELECTED); }
		void selectNone()                        { ListView_SetItemState(_list->hWnd(), -1, 0, LVIS_SELECTED); }
		void removeSelected();
		std::vector<Item> getSelected() const;
		Item getFocused() const                  { return Item(ListView_GetNextItem(_list->hWnd(), -1, LVNI_FOCUSED), _list); }
	};
private:
	int _ctxMenuId;
public:
	ItemsProxy items;
	enum class View { VW_DETAILS=LV_VIEW_DETAILS, VW_ICON=LV_VIEW_ICON,
		VW_LIST=LV_VIEW_LIST, VW_SMALLICON=LV_VIEW_SMALLICON, VW_TILE=LV_VIEW_TILE };

	ListView()                      : items(nullptr) { }
	ListView(HWND hwnd)             : ListView() { operator=(hwnd); }
	ListView(const Window& wnd)     : ListView() { operator=(wnd); }
	ListView(const ListView& other) : ListView() { operator=(other); }

	ListView& operator=(HWND hwnd);
	ListView& operator=(const Window& wnd)     { return operator=(wnd.hWnd()); }
	ListView& operator=(const ListView& other) { operator=(other.hWnd()); _ctxMenuId = other._ctxMenuId; return *this; }

	ListView& create(WindowPopup *parent, int id, POINT pos, SIZE size);
	ListView& setFullRowSelect()         { ListView_SetExtendedListViewStyle(hWnd(), LVS_EX_FULLROWSELECT); return *this; }
	ListView& setRedraw(bool doRedraw)   { sendMessage(WM_SETREDRAW, static_cast<WPARAM>(static_cast<BOOL>(doRedraw)), 0); return *this; }
	ListView& setContextMenu(int menuId) { _ctxMenuId = menuId; return *this; }
	ListView& setView(View view)         { ListView_SetView(hWnd(), static_cast<DWORD>(view)); return *this; }
	View      getView() const            { return static_cast<View>(ListView_GetView(hWnd())); }
	
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

}//namespace c4w