
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
		int index;
		Item(int itemIndex, ListView *pList);
		Item();
		void         remove();
		void         swapWith(size_t itemIndex);
		Item&        ensureVisible();
		bool         isVisible() const;
		Item&        setSelect(bool select);
		bool         isSelected() const;
		Item&        setFocus();
		bool         isFocused() const;
		RECT         getRect() const;
		std::wstring getText(size_t columnIndex = 0) const;
		Item&        setText(const wchar_t *text, size_t columnIndex = 0);
		Item&        setText(const std::wstring& text, size_t columnIndex = 0);
		LPARAM       getParam() const;
		Item&        setParam(LPARAM lp);
		int          getIcon() const;
		Item&        setIcon(int imagelistIconIndex);
	};

	class Collection final {
	private:
		ListView *_list;
	public:
		explicit Collection(ListView *pList);
		Item operator[](size_t itemIndex);
		int               count() const;
		Item              add(const wchar_t *caption, int imagelistIconIndex = -1, int positionIndex = -1);
		Item              add(const std::wstring& caption, int imagelistIconIndex = -1, int positionIndex = -1);
		std::vector<Item> getAll() const;
		void              removeAll();
		Item              find(const wchar_t *caption);
		Item              find(const std::wstring& caption);
		bool              exists(const wchar_t *caption);
		bool              exists(const std::wstring& caption);
		int               countSelected() const;
		void              select(const std::vector<size_t>& indexes);
		void              selectAll();
		void              selectNone();
		void              removeSelected();
		std::vector<Item> getSelected() const;
		Item              getFocused() const;
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

	~ListView();
	ListView();
	ListView(HWND hWnd);
	ListView& operator=(HWND hwnd);

	HWND      hWnd() const;
	ListView& create(HWND hParent, int id, POINT pos, SIZE size, View view = View::DETAILS);
	ListView& setContextMenu(int contextMenuId);
	ListView& setFullRowSelect();
	ListView& setRedraw(bool doRedraw);
	ListView& focus();
	ListView& setView(View view);
	View      getView() const;
	ListView& iconPush(int iconId);
	ListView& iconPush(const wchar_t *fileExtension);
	int       columnCount() const;
	ListView& columnAdd(const wchar_t *caption, int cx);
	ListView& columnFit(size_t columnIndex);

	static std::vector<std::wstring> getAllText(std::vector<Item> items, size_t columnIndex = 0);
private:
	HIMAGELIST _proceedImagelist();
	int        _showContextMenu(bool followCursor);
	static LRESULT CALLBACK _proc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp,
		UINT_PTR idSubclass, DWORD_PTR refData);
};