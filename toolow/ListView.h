//
// ListView control handling.
// Dismembered into subclasses at night of Friday, December 7, 2012.
// Part of TOOLOW - Thin Object Oriented Layer Over Win32.
//

#pragma once
#include "Window.h"
#include <CommCtrl.h>

class ListView : public Window {
public:
	class Item {
	public:
		int i;
		Item()                           : i(-1), _list(NULL) { }
		Item(int index, ListView *pList) : i(index), _list(pList) { }
		void     remove()                { ListView_DeleteItem(_list->hWnd(), i); }
		void     swapWith(int index);
		Item&    ensureVisible();
		bool     isVisible() const       { return ListView_IsItemVisible(_list->hWnd(), i) == TRUE; }
		Item&    setSelect(bool select)  { ListView_SetItemState(_list->hWnd(), i, select ? LVIS_SELECTED : 0, LVIS_SELECTED); return *this; }
		bool     isSelected() const      { return (ListView_GetItemState(_list->hWnd(), i, LVIS_SELECTED) & LVIS_SELECTED) != 0; }
		Item&    setFocus()              { ListView_SetItemState(_list->hWnd(), i, LVIS_FOCUSED, LVIS_FOCUSED); return *this; }
		bool     isFocused() const       { return (ListView_GetItemState(_list->hWnd(), i, LVIS_FOCUSED) & LVIS_FOCUSED) != 0; }
		Item&    getRect(RECT *rc)       { ListView_GetItemRect(_list->hWnd(), i, rc, LVIR_BOUNDS); return *this; }
		wchar_t* getText(wchar_t *pBuf, int szBuf, int iCol=0) const { ListView_GetItemText(_list->hWnd(), i, iCol, pBuf, szBuf); return pBuf; }
		String*  getText(String *pBuf, int iCol=0) const;
		String   getText(int iCol=0) const                           { String ret; getText(&ret, iCol); return ret; }
		Item&    setText(const wchar_t *text, int iCol=0)            { ListView_SetItemText(_list->hWnd(), i, iCol, (wchar_t*)text); return *this; }
		LPARAM   getParam() const;
		Item&    setParam(LPARAM lp);
		int      getIcon() const;
		Item&    setIcon(int iconIdx);
	private:
		ListView *_list;
	};

	class ItemsProxy {
	public:
		explicit ItemsProxy(ListView *pList) : _list(pList) { }
		Item        operator[](int index)          { return Item(index, _list); }
		int         count() const                  { return ListView_GetItemCount(_list->hWnd()); }
		Item        add(const wchar_t *caption, int iconIdx=-1, int at=-1);
		Array<Item> getAll() const;
		void        removeAll()                    { ListView_DeleteAllItems(_list->hWnd()); }
		Item        find(const wchar_t *caption);
		bool        exists(const wchar_t *caption) { return find(caption).i != -1; }
		int         countSelected() const          { return ListView_GetSelectedCount(_list->hWnd()); }
		void        select(const Array<int> *idx);
		void        selectAll()                    { ListView_SetItemState(_list->hWnd(), -1, LVIS_SELECTED, LVIS_SELECTED); }
		void        selectNone()                   { ListView_SetItemState(_list->hWnd(), -1, 0, LVIS_SELECTED); }
		void        removeSelected();
		Array<Item> getSelected() const;
		Item        getFocused() const             { return Item(ListView_GetNextItem(_list->hWnd(), -1, LVNI_FOCUSED), _list); }
	private:
		ListView *_list;
	};

public:
	ItemsProxy items;
	enum class View { VW_DETAILS=LV_VIEW_DETAILS, VW_ICON=LV_VIEW_ICON, VW_LIST=LV_VIEW_LIST, VW_SMALLICON=LV_VIEW_SMALLICON, VW_TILE=LV_VIEW_TILE };

	ListView()                      : items(NULL), _ctxMenuId(0) { }
	ListView(HWND hwnd)             : items(NULL), _ctxMenuId(0) { operator=(hwnd); }
	ListView(const Window& wnd)     : items(NULL), _ctxMenuId(0) { operator=(wnd); }
	ListView(const ListView& other) : items(NULL), _ctxMenuId(0) { operator=(other); }

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

	int       columnCount() const { return Header_GetItemCount(ListView_GetHeader(hWnd())); }
	ListView& columnAdd(const wchar_t *caption, int cx);
	ListView& columnFit(int iCol);

private:
	int _ctxMenuId;

	HIMAGELIST _proceedImageList();
	int _showCtxMenu(bool followCursor);
	static LRESULT CALLBACK _Proc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp, UINT_PTR idSubclass, DWORD_PTR refData);
};