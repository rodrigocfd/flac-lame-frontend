/**
 * Part of WinUtil - Windows Utilities
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winutil
 */

#pragma once
#include <vector>
#include "menu.h"
#include <CommCtrl.h>

namespace winutil {

class listview final {
public:
	class item final {
	private:
		listview *_list;
	public:
		static const size_t npos = -1;
		size_t index;

		item(size_t itemIndex, listview *pList) : index(itemIndex), _list(pList) { }
		item() : item(item::npos, nullptr) { }
		
		void         remove()           { ListView_DeleteItem(_list->_hWnd, index); }
		void         swap_with(size_t itemIndex);
		item&        ensure_visible();
		bool         is_visible() const { return ListView_IsItemVisible(_list->_hWnd, index) == TRUE; }
		item&        set_select(bool select);
		bool         is_selected() const;
		item&        focus();
		bool         is_focused() const;
		RECT         get_rect() const;
		std::wstring get_text(size_t columnIndex = 0) const;
		item&        set_text(const wchar_t *text, size_t columnIndex = 0);
		item&        set_text(const std::wstring& text, size_t columnIndex = 0) { return set_text(text.c_str(), columnIndex); }
		LPARAM       get_param() const;
		item&        set_param(LPARAM lp);
		int          get_icon_index() const;
		item&        set_icon_index(int imagelistIconIndex);
	};

	class collection final {
	private:
		listview *_list;
	public:
		explicit collection(listview *pList) : _list(pList) { }

		item operator[](size_t itemIndex) { return item(static_cast<int>(itemIndex), _list); }
		size_t            count() const   { return ListView_GetItemCount(_list->_hWnd); }
		item              add(const wchar_t *caption, int imagelistIconIndex = -1, size_t positionIndex = item::npos);
		item              add(const std::wstring& caption, int imagelistIconIndex = -1, size_t positionIndex = item::npos) { return add(caption.c_str(), imagelistIconIndex, positionIndex); }
		std::vector<item> get_all() const;
		void              remove_all()    { ListView_DeleteAllItems(_list->_hWnd); }
		item              find(const wchar_t *caption);
		item              find(const std::wstring& caption)   { return find(caption.c_str()); }
		bool              exists(const wchar_t *caption)      { return find(caption).index != item::npos; }
		bool              exists(const std::wstring& caption) { return exists(caption.c_str()); }
		size_t            count_selected() const              { return ListView_GetSelectedCount(_list->_hWnd); }
		void              select(const std::vector<size_t>& indexes);
		void              select_all()    { ListView_SetItemState(_list->_hWnd, -1, LVIS_SELECTED, LVIS_SELECTED); }
		void              select_none()   { ListView_SetItemState(_list->_hWnd, -1, 0, LVIS_SELECTED); }
		void              remove_selected();
		std::vector<item> get_selected() const;
		item              get_focused() const { return item(ListView_GetNextItem(_list->_hWnd, -1, LVNI_FOCUSED), _list); }
	};

	enum class view : WORD {
		DETAILS   = LV_VIEW_DETAILS,
		ICON      = LV_VIEW_ICON,
		LIST      = LV_VIEW_LIST,
		SMALLICON = LV_VIEW_SMALLICON,
		TILE      = LV_VIEW_TILE
	};
private:
	HWND _hWnd;
	menu _contextMenu;
public:
	collection items;

	~listview()                { _contextMenu.destroy(); }
	listview()                 : _hWnd(nullptr), items(this) { }
	listview(HWND hwnd)        : listview() { operator=(hwnd); }
	listview(const listview&)  = delete;
	listview(listview&& other) : listview() { operator=(std::move(other)); }

	listview& operator=(HWND hWnd);
	listview& operator=(const listview&) = delete;
	listview& operator=(listview&&);

	HWND      hwnd() const         { return _hWnd; }
	listview& create(HWND hParent, int id, POINT pos, SIZE size, view viewType = view::DETAILS);
	listview& set_context_menu(int contextMenuId);
	listview& set_full_row_select();
	listview& set_redraw(bool doRedraw);
	listview& focus();
	listview& set_view(view viewType);
	view      get_view() const     { return static_cast<view>(ListView_GetView(_hWnd)); }
	listview& icon_push(int iconId);
	listview& icon_push(const wchar_t *fileExtension);
	size_t    column_count() const { return Header_GetItemCount(ListView_GetHeader(_hWnd)); }
	listview& column_add(const wchar_t *caption, int cx);
	listview& column_fit(size_t columnIndex);

	static std::vector<std::wstring> get_all_text(std::vector<item> items, size_t columnIndex = 0);
private:
	HIMAGELIST _proceed_imagelist();
	int        _show_context_menu(bool followCursor);
	static LRESULT CALLBACK _proc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp, UINT_PTR idSubclass, DWORD_PTR refData);
};

}//namespace winutil