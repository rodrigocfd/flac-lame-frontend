/*!
 * @file
 * @brief Often-used controls.
 * @details Part of WOLF - Win32 Object Lambda Framework.
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/wolf
 */

#pragma once
#include "res.h"
#include "str.h"
#include "wnd.h"

namespace wolf {
namespace ctrl {

/// Fit size and position of a bunch of controls inside a window.
class Resizer final {
public:
	enum class Do {
		REPOS,  // control size is fixed; control moves around anchored
		RESIZE, // control size stretches; control doesn't move
		NOTHING // control doesn't move or resize
	};
private:
	struct _Ctrl final {
		wnd::Wnd wnd;      // handle to child window
		RECT     rcOrig;   // original coordinates relative to parent
		Do       modeHorz; // horizontal mode
		Do       modeVert; // vertical mode
	};

private:
	std::vector<_Ctrl> _ctrls;
	SIZE _szOrig;
	std::function<void()> _afterResize;
public:
	Resizer& add(std::initializer_list<int> ctrlIds, wnd::Wnd *parent, Do modeHorz, Do modeVert);
	Resizer& add(std::initializer_list<wnd::Wnd> children, Do modeHorz, Do modeVert);
	Resizer& afterResize(std::function<void()> callback) { _afterResize = std::move(callback); return *this; }
private:
	void _addOne(wnd::Wnd ctrl, Do modeHorz, Do modeVert);
	static LRESULT CALLBACK _Proc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp, UINT_PTR idSubclass, DWORD_PTR refData);
};


/// Status bar automation.
class StatusBar final {
private:
	struct _Part final { BYTE sizePixels; float resizeWeight; };
private:
	wnd::Wnd           _sb;
	std::vector<_Part> _parts;
	std::vector<int>   _rightEdges;
public:
	StatusBar& create(HWND hParent, int numPartsItWillHave);
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


/// Ordinary textbox with some utilities.
class TextBox : public wnd::Child {
private:
	res::Font _font;
	UINT _notifyKeyUp;
	std::function<void(WORD)> _onKeyUp;
public:
	TextBox()                     : _notifyKeyUp(0) { }
	TextBox(HWND hwnd)            { operator=(hwnd); }
	TextBox(const wnd::Wnd& wnd)  { operator=(wnd); }
	TextBox(const TextBox& other) { operator=(other); }

	TextBox& operator=(HWND hwnd);
	TextBox& operator=(const wnd::Wnd& wnd)              { return operator=(wnd.hWnd()); }
	TextBox& operator=(const TextBox& other)             { return operator=(other.hWnd()); }
	TextBox& create(wnd::Wnd *parent, int id, POINT pos, int cx, UINT extraStyles=0);
	int      getTextLen() const                          { return ::GetWindowTextLength(hWnd()); }
	std::vector<std::wstring> getTextLines() const       { return str::Explode(getText(), L"\r\n"); }
	TextBox& setFont(const res::Font& font);
	const res::Font& getFont() const                     { return _font; }
	TextBox& selSetAll()                                 { sendMessage(EM_SETSEL, 0, -1); return *this; }
	TextBox& selSet(int start, int length)               { sendMessage(EM_SETSEL, start, start + length); return *this; }
	void     selGet(int *start, int *length);
	TextBox& selReplace(const wchar_t *text)             { sendMessage(EM_REPLACESEL, TRUE, reinterpret_cast<LPARAM>(text)); return *this; }
	TextBox& selReplace(const std::wstring text)         { return selReplace(text.c_str()); }
	void     onKeyUp(std::function<void(WORD)> callback) { _onKeyUp = callback; }
private:
	static LRESULT CALLBACK _Proc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp, UINT_PTR idSubclass, DWORD_PTR refData);
};


/// Ordinary combobox control.
class Combo : public wnd::Child {
public:
	Combo()                    { }
	Combo(HWND hwnd)           { operator=(hwnd); }
	Combo(const wnd::Wnd& wnd) { operator=(wnd); }
	Combo(const Combo& other)  { operator=(other); }

	Combo& operator=(HWND hwnd)           { static_cast<wnd::Wnd*>(this)->operator=(hwnd); return *this; }
	Combo& operator=(const wnd::Wnd& wnd) { return operator=(wnd.hWnd()); }
	Combo& operator=(const Combo& other)  { return operator=(other.hWnd()); }
	Combo& create(wnd::Wnd *parent, int id, POINT pos, int cx);
	int    itemCount() const              { return static_cast<int>(sendMessage(CB_GETCOUNT, 0, 0)); }
	Combo& itemRemoveAll()                { sendMessage(CB_RESETCONTENT, 0, 0); return *this; }
	Combo& itemSetSelected(int i)         { sendMessage(CB_SETCURSEL, i, 0); return *this; }
	int    itemGetSelected() const        { return static_cast<int>(sendMessage(CB_GETCURSEL, 0, 0)); }
	Combo& itemAdd(std::initializer_list<const wchar_t*> arrStr);
	wchar_t*     itemGetText(int i, wchar_t *pBuf, int szBuf) const;
	std::wstring itemGetText(int i) const;
	std::wstring itemGetSelectedText() const { return itemGetText(itemGetSelected()); }
};


/// Ordinary listbox control.
class ListBox : public wnd::Child {
public:
	ListBox()                     { }
	ListBox(HWND hwnd)            { operator=(hwnd); }
	ListBox(const wnd::Wnd& wnd)  { operator=(wnd); }
	ListBox(const ListBox& other) { operator=(other); }

	ListBox& operator=(HWND hwnd)            { static_cast<wnd::Wnd*>(this)->operator=(hwnd); return *this; }
	ListBox& operator=(const wnd::Wnd& wnd)  { return operator=(wnd.hWnd()); }
	ListBox& operator=(const ListBox& other) { return operator=(other.hWnd()); }
	ListBox& create(wnd::Wnd *parent, int id, POINT pos, SIZE size);
	ListBox& itemAdd(std::initializer_list<const wchar_t*> arrStr);
	int      itemCount() const               { return static_cast<int>(sendMessage(LB_GETCOUNT, 0, 0)); }
	int      itemCountSelected() const;
	int      itemGetSelected(std::vector<int> *indexesBuf=nullptr) const;
	wchar_t*     itemGetText(int i, wchar_t *pBuf, int szBuf) const;
	std::wstring itemGetText(int i) const;
	ListBox& itemRemoveAll()                 { sendMessage(LB_RESETCONTENT, 0, 0); return *this; }
};


/// Ordinary radio button control.
class Radio : public wnd::Child {
public:
	enum class EmulateClick { YES, NO };

	Radio()                    { }
	Radio(HWND hwnd)           { operator=(hwnd); }
	Radio(const wnd::Wnd& wnd) { operator=(wnd); }
	Radio(const Radio& other)  { operator=(other); }

	Radio& operator=(HWND hwnd)           { static_cast<wnd::Wnd*>(this)->operator=(hwnd); return *this; }
	Radio& operator=(const wnd::Wnd& wnd) { return operator=(wnd.hWnd()); }
	Radio& operator=(const Radio& other)  { return operator=(other.hWnd()); }
	Radio& create(wnd::Wnd *parent, int id, const wchar_t *caption, bool beginGroup, POINT pos, SIZE size);
	bool   isChecked()                    { return sendMessage(BM_GETCHECK, 0, 0) == BST_CHECKED; }
	void   setCheck(bool checked, EmulateClick emulateClick);
};


/// Ordinary checkbox control.
class CheckBox : public Radio {
public:
	CheckBox()                      { }
	CheckBox(HWND hwnd)             { operator=(hwnd); }
	CheckBox(const wnd::Wnd& wnd)   { operator=(wnd); }
	CheckBox(const CheckBox& other) { operator=(other); }

	CheckBox& operator=(HWND hwnd)             { static_cast<wnd::Wnd*>(this)->operator=(hwnd); return *this; }
	CheckBox& operator=(const wnd::Wnd& wnd)   { return operator=(wnd.hWnd()); }
	CheckBox& operator=(const CheckBox& other) { return operator=(other.hWnd()); }
	CheckBox& create(wnd::Wnd *parent, int id, const wchar_t *caption, POINT pos, SIZE size);
};


/// Ordinary progress bar control.
class ProgressBar final : public wnd::Child {
public:
	ProgressBar()                         { }
	ProgressBar(HWND hwnd)                { operator=(hwnd); }
	ProgressBar(const wnd::Wnd& wnd)      { operator=(wnd); }
	ProgressBar(const ProgressBar& other) { operator=(other); }

	ProgressBar& operator=(HWND hwnd)                   { static_cast<wnd::Wnd*>(this)->operator=(hwnd); return *this; }
	ProgressBar& operator=(const wnd::Wnd& wnd)         { return operator=(wnd.hWnd()); }
	ProgressBar& operator=(const ProgressBar& other)    { return operator=(other.hWnd()); }
	ProgressBar& create(wnd::Wnd *parent, int id, POINT pos, SIZE size);
	ProgressBar& setRange(int minVal, int maxVal)       { sendMessage(PBM_SETRANGE, 0, MAKELPARAM(minVal, maxVal)); return *this; }
	ProgressBar& setRange(int minVal, size_t maxVal)    { return setRange(minVal, static_cast<int>(maxVal)); }
	ProgressBar& setPos(int pos)                        { sendMessage(PBM_SETPOS, pos, 0); return *this; }
	ProgressBar& setPos(size_t pos)                     { return setPos(static_cast<int>(pos)); }
	ProgressBar& setPos(double pos)                     { return setPos(static_cast<int>(pos + 0.5)); }
	int          getPos()                               { return static_cast<int>(sendMessage(PBM_GETPOS, 0, 0)); }
	ProgressBar& animateMarquee(bool animate);
};


/// Ordinary listview control.
class ListView final : public wnd::Child {
public:
	/// A single listview item.
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

	/// Provides operations to all items of a listview control.
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
public:
	ItemsProxy items;
	res::MenuContext contextMenu;
	enum class View { VW_DETAILS=LV_VIEW_DETAILS, VW_ICON=LV_VIEW_ICON,
		VW_LIST=LV_VIEW_LIST, VW_SMALLICON=LV_VIEW_SMALLICON, VW_TILE=LV_VIEW_TILE };

	ListView()                      : items(nullptr) { }
	ListView(HWND hwnd)             : ListView() { operator=(hwnd); }
	ListView(const wnd::Wnd& wnd)   : ListView() { operator=(wnd); }
	ListView(const ListView& other) : ListView() { operator=(other); }

	ListView& operator=(HWND hwnd);
	ListView& operator=(const wnd::Wnd& wnd)     { return operator=(wnd.hWnd()); }
	ListView& operator=(const ListView& other)   { operator=(other.hWnd()); contextMenu = other.contextMenu; return *this; }

	ListView& create(Wnd *parent, int id, POINT pos, SIZE size);
	ListView& setFullRowSelect()                 { ListView_SetExtendedListViewStyle(hWnd(), LVS_EX_FULLROWSELECT); return *this; }
	ListView& setRedraw(bool doRedraw)           { sendMessage(WM_SETREDRAW, static_cast<WPARAM>(static_cast<BOOL>(doRedraw)), 0); return *this; }
	ListView& setView(View view)                 { ListView_SetView(hWnd(), static_cast<DWORD>(view)); return *this; }
	View      getView() const                    { return static_cast<View>(ListView_GetView(hWnd())); }
	
	ListView& iconPush(int iconId);
	ListView& iconPush(const wchar_t *fileExtension);

	int       columnCount() const                { return Header_GetItemCount(ListView_GetHeader(hWnd())); }
	ListView& columnAdd(const wchar_t *caption, int cx);
	ListView& columnFit(int iCol);

	static std::vector<std::wstring> TextsFromItems(std::vector<Item> items, int iCol=0);
private:
	HIMAGELIST _proceedImageList();
	int        _showContextMenu(bool followCursor);
	static LRESULT CALLBACK _Proc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp, UINT_PTR idSubclass, DWORD_PTR refData);
};

}//namespace ctrl
}//namespace wolf