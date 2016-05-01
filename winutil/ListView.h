
#pragma once
#include <vector>
#include "Menu.h"
#include <CommCtrl.h>

class ListView final {
public:
	class Item final {
	private:
		ListView *_list;
	public:
		static const size_t npos = -1;
		size_t index;

		Item(size_t itemIndex, ListView *pList) : index(itemIndex), _list(pList) { }
		Item() : Item(Item::npos, nullptr) { }
		
		void         remove()          { ListView_DeleteItem(_list->_hWnd, index); }
		void         swapWith(size_t itemIndex);
		Item&        ensureVisible();
		bool         isVisible() const { return ListView_IsItemVisible(_list->_hWnd, index) == TRUE; }
		Item&        setSelect(bool select);
		bool         isSelected() const;
		Item&        focus();
		bool         isFocused() const;
		RECT         getRect() const;
		std::wstring getText(size_t columnIndex = 0) const;
		Item&        setText(const wchar_t *text, size_t columnIndex = 0);
		Item&        setText(const std::wstring& text, size_t columnIndex = 0) { return setText(text.c_str(), columnIndex); }
		LPARAM       getParam() const;
		Item&        setParam(LPARAM lp);
		int          getIconIndex() const;
		Item&        setIconIndex(int imagelistIconIndex);
	};

	class Collection final {
	private:
		ListView *_list;
	public:
		explicit Collection(ListView *pList) : _list(pList) { }

		Item operator[](size_t itemIndex) { return Item(static_cast<int>(itemIndex), _list); }
		size_t            count() const   { return ListView_GetItemCount(_list->_hWnd); }
		Item              add(const wchar_t *caption, int imagelistIconIndex = -1, size_t positionIndex = Item::npos);
		Item              add(const std::wstring& caption, int imagelistIconIndex = -1, size_t positionIndex = Item::npos) { return add(caption.c_str(), imagelistIconIndex, positionIndex); }
		std::vector<Item> getAll() const;
		void              removeAll()     { ListView_DeleteAllItems(_list->_hWnd); }
		Item              find(const wchar_t *caption);
		Item              find(const std::wstring& caption)   { return find(caption.c_str()); }
		bool              exists(const wchar_t *caption)      { return find(caption).index != Item::npos; }
		bool              exists(const std::wstring& caption) { return exists(caption.c_str()); }
		size_t            countSelected() const               { return ListView_GetSelectedCount(_list->_hWnd); }
		void              select(const std::vector<size_t>& indexes);
		void              selectAll()     { ListView_SetItemState(_list->_hWnd, -1, LVIS_SELECTED, LVIS_SELECTED); }
		void              selectNone()    { ListView_SetItemState(_list->_hWnd, -1, 0, LVIS_SELECTED); }
		void              removeSelected();
		std::vector<Item> getSelected() const;
		Item              getFocused() const { return Item(ListView_GetNextItem(_list->_hWnd, -1, LVNI_FOCUSED), _list); }
	};

	enum class View : WORD {
		DETAILS   = LV_VIEW_DETAILS,
		ICON      = LV_VIEW_ICON,
		LIST      = LV_VIEW_LIST,
		SMALLICON = LV_VIEW_SMALLICON,
		TILE      = LV_VIEW_TILE
	};
private:
	HWND _hWnd;
	Menu _contextMenu;
public:
	Collection items;

	~ListView()                { _contextMenu.destroy(); }
	ListView()                 : _hWnd(nullptr), items(this) { }
	ListView(HWND hwnd)        : ListView() { operator=(hwnd); }
	ListView(const ListView&)  = delete;
	ListView(ListView&& other) : ListView() { operator=(std::move(other)); }

	ListView& operator=(HWND hWnd);
	ListView& operator=(const ListView&) = delete;
	ListView& operator=(ListView&&);

	HWND      hWnd() const        { return _hWnd; }
	ListView& create(HWND hParent, int id, POINT pos, SIZE size, View view = View::DETAILS);
	ListView& setContextMenu(int contextMenuId);
	ListView& setFullRowSelect();
	ListView& setRedraw(bool doRedraw);
	ListView& focus();
	ListView& setView(View view);
	View      getView() const     { return static_cast<View>(ListView_GetView(_hWnd)); }
	ListView& iconPush(int iconId);
	ListView& iconPush(const wchar_t *fileExtension);
	size_t    columnCount() const { return Header_GetItemCount(ListView_GetHeader(_hWnd)); }
	ListView& columnAdd(const wchar_t *caption, int cx);
	ListView& columnFit(size_t columnIndex);

	static std::vector<std::wstring> getAllText(std::vector<Item> items, size_t columnIndex = 0);
private:
	HIMAGELIST _proceedImagelist();
	int        _showContextMenu(bool followCursor);
	static LRESULT CALLBACK _proc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp, UINT_PTR idSubclass, DWORD_PTR refData);
};