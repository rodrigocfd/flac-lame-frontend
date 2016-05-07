/**
 * Part of WinUtil - Windows Utilities
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winutil
 */

#include "listview.h"
#include "icon.h"
#include "str.h"
#include "sys.h"
using namespace winutil;
using std::vector;
using std::wstring;

void listview::item::swap_with(size_t itemIndex)
{
	item newItem = _list->items[itemIndex];

	size_t numCols = _list->column_count();
	wstring tmpstr;
	for (size_t c = 0; c < numCols; ++c) { // swap texts of all columns
		tmpstr = get_text(c);
		set_text(newItem.get_text(c), c);
		newItem.set_text(tmpstr, c);
	}

	LPARAM oldp = get_param(); // swap LPARAMs
	set_param(newItem.get_param());
	newItem.set_param(oldp);

	int oldi = get_icon_index(); // swap icons
	set_icon_index(newItem.get_icon_index());
	newItem.set_icon_index(oldi);
}

listview::item& listview::item::ensure_visible()
{
	if (_list->get_view() == view::DETAILS) {
		// In details view, ListView_EnsureVisible() won't center the item vertically.
		// This new implementation has this behavior.
		RECT rc = { 0 };
		GetClientRect(_list->_hWnd, &rc);
		int cyList = rc.bottom; // total height of list

		SecureZeroMemory(&rc, sizeof(rc));
		LVITEMINDEX lvii = { 0 };
		lvii.iItem = ListView_GetTopIndex(_list->_hWnd); // 1st visible item
		ListView_GetItemIndexRect(_list->_hWnd, &lvii, 0, LVIR_BOUNDS, &rc);
		int cyItem = rc.bottom - rc.top; // height of a single item
		int xTop = rc.top; // topmost X of 1st visible item

		SecureZeroMemory(&rc, sizeof(rc));
		SecureZeroMemory(&lvii, sizeof(lvii));
		lvii.iItem = static_cast<int>(index);
		ListView_GetItemIndexRect(_list->_hWnd, &lvii, 0, LVIR_BOUNDS, &rc);
		int xUs = rc.top; // our current X

		if (xUs < xTop || xUs > xTop + cyList) { // if we're not visible
			ListView_Scroll(_list->_hWnd, 0, xUs - xTop - cyList / 2 + cyItem * 2);
		}
	} else {
		ListView_EnsureVisible(_list->_hWnd, index, FALSE);
	}
	return *this;
}

listview::item& listview::item::set_select(bool select)
{
	ListView_SetItemState(_list->_hWnd, index,
		select ? LVIS_SELECTED : 0, LVIS_SELECTED);
	return *this;
}

bool listview::item::is_selected() const
{
	return (ListView_GetItemState(_list->_hWnd,
		index, LVIS_SELECTED) & LVIS_SELECTED) != 0;
}

listview::item& listview::item::focus()
{
	ListView_SetItemState(_list->_hWnd,
		index, LVIS_FOCUSED, LVIS_FOCUSED);
	return *this;
}

bool listview::item::is_focused() const
{
	return (ListView_GetItemState(_list->_hWnd,
		index, LVIS_FOCUSED) & LVIS_FOCUSED) != 0;
}

RECT listview::item::get_rect() const
{
	RECT rc = { 0 };
	ListView_GetItemRect(_list->_hWnd, index, &rc, LVIR_BOUNDS);
	return rc;
}

wstring listview::item::get_text(size_t columnIndex) const
{
	// http://forums.codeguru.com/showthread.php?351972-Getting-listView-item-text-length
	LVITEM lvi = { 0 };
	lvi.iItem = static_cast<int>(index);
	lvi.iSubItem = static_cast<int>(columnIndex);

	// Notice that, since strings' size always increase, if the buffer
	// was previously allocated with a value bigger than our 1st step,
	// this will speed up the size checks.

	wstring buf(64, L'\0'); // speed-up 1st allocation
	int baseBufLen = 0;
	int charsWrittenWithoutNull = 0;
	do {
		baseBufLen += 64; // buffer increasing step, arbitrary!
		buf.resize(baseBufLen);
		lvi.cchTextMax = baseBufLen;
		lvi.pszText = &buf[0];
		charsWrittenWithoutNull = static_cast<int>(
			SendMessage(_list->_hWnd, LVM_GETITEMTEXT,
				index, reinterpret_cast<LPARAM>(&lvi)) );
	} while (charsWrittenWithoutNull == baseBufLen - 1); // to break, must have at least 1 char gap

	str::trim_nulls(buf);
	return buf;
}

listview::item& listview::item::set_text(const wchar_t *text, size_t columnIndex)
{
	ListView_SetItemText(_list->_hWnd, index,
		static_cast<int>(columnIndex), const_cast<wchar_t*>(text));
	return *this;
}

LPARAM listview::item::get_param() const
{
	LVITEM lvi = { 0 };
	lvi.iItem = static_cast<int>(index);
	lvi.mask = LVIF_PARAM;

	ListView_GetItem(_list->_hWnd, &lvi);
	return lvi.lParam;
}

listview::item& listview::item::set_param(LPARAM lp)
{
	LVITEM lvi = { 0 };
	lvi.iItem = static_cast<int>(index);
	lvi.mask = LVIF_PARAM;
	lvi.lParam = lp;

	ListView_SetItem(_list->_hWnd, &lvi);
	return *this;
}

int listview::item::get_icon_index() const
{
	LVITEM lvi = { 0 };
	lvi.iItem = static_cast<int>(index);
	lvi.mask = LVIF_IMAGE;

	ListView_GetItem(_list->_hWnd, &lvi);
	return lvi.iImage; // return index of icon within imagelist
}

listview::item& listview::item::set_icon_index(int imagelistIconIndex)
{
	LVITEM lvi = { 0 };
	lvi.iItem = static_cast<int>(index);
	lvi.mask = LVIF_IMAGE;
	lvi.iImage = imagelistIconIndex;

	ListView_SetItem(_list->_hWnd, &lvi);
	return *this;
}


listview::item listview::collection::add(const wchar_t *caption, int imagelistIconIndex, size_t positionIndex)
{
	LVITEM lvi = { 0 };
	lvi.iItem = static_cast<int>(positionIndex == -1 ? 0x0FFFFFFF : positionIndex);
	lvi.mask = LVIF_TEXT | (imagelistIconIndex == -1 ? 0 : LVIF_IMAGE);
	lvi.pszText = const_cast<wchar_t*>(caption);
	lvi.iImage = imagelistIconIndex;

	return item(ListView_InsertItem(_list->_hWnd, &lvi), _list); // return index of newly inserted item
}

vector<listview::item> listview::collection::get_all() const
{
	size_t totItems = count();

	vector<item> items; // a big array with all items in list
	items.reserve(totItems);

	for (size_t i = 0; i < totItems; ++i) {
		items.emplace_back(i, _list);
	}
	return items;
}

listview::item listview::collection::find(const wchar_t *caption)
{
	LVFINDINFO lfi = { 0 };
	lfi.flags = LVFI_STRING; // search is case-insensitive
	lfi.psz = caption;

	return item(ListView_FindItem(_list->_hWnd, -1, &lfi), _list); // returns -1 if not found
}

void listview::collection::select(const vector<size_t>& indexes)
{
	// Select the items whose indexes have been passed in the array.
	for (const size_t& index : indexes) {
		ListView_SetItemState(_list->_hWnd,
			static_cast<int>(index), LVIS_SELECTED, LVIS_SELECTED);
	}
}

void listview::collection::remove_selected()
{
	_list->set_redraw(false);
	int i = -1;
	while ((i = ListView_GetNextItem(_list->_hWnd, -1, LVNI_SELECTED)) != -1) {
		ListView_DeleteItem(_list->_hWnd, i);
	}
	_list->set_redraw(true);
}

vector<listview::item> listview::collection::get_selected() const
{
	vector<item> items;
	items.reserve(count_selected());

	int iBase = -1;
	for (;;) {
		iBase = ListView_GetNextItem(_list->_hWnd, iBase, LVNI_SELECTED);
		if (iBase == -1) break;
		items.emplace_back(iBase, _list);
	}
	return items;
}


const int IDSUBCLASS = 1;

listview& listview::operator=(HWND hwnd)
{
	if (_hWnd != hwnd) {
		if (_hWnd) RemoveWindowSubclass(_hWnd, _proc, IDSUBCLASS);
		_hWnd = hwnd;
		SetWindowSubclass(_hWnd, _proc, IDSUBCLASS, reinterpret_cast<DWORD_PTR>(this));
		_contextMenu.destroy();
	}
	return *this;
}

listview& listview::operator=(listview&& other)
{
	operator=(other._hWnd);
	other._hWnd = nullptr;
	_contextMenu = std::move(other._contextMenu);
	return *this;
}

listview& listview::create(HWND hParent, int id, POINT pos, SIZE size, view viewType)
{
	// For children, WS_BORDER gives old, flat drawing; always use WS_EX_CLIENTEDGE.
	return operator=( CreateWindowEx(WS_EX_CLIENTEDGE, WC_LISTVIEW, nullptr,
		WS_CHILD | WS_VISIBLE | WS_TABSTOP | static_cast<DWORD>(viewType),
		pos.x, pos.y, size.cx, size.cy,
		hParent, reinterpret_cast<HMENU>(static_cast<UINT_PTR>(id)),
		reinterpret_cast<HINSTANCE>(GetWindowLongPtr(hParent, GWLP_HINSTANCE)),
		nullptr) );
}

listview& listview::set_context_menu(int contextMenuId)
{
	_contextMenu.load_resource(contextMenuId, 0,
		reinterpret_cast<HINSTANCE>(GetWindowLongPtr(_hWnd, GWLP_HINSTANCE)));
	return *this;
}

listview& listview::set_full_row_select()
{
	ListView_SetExtendedListViewStyle(_hWnd, LVS_EX_FULLROWSELECT);
	return *this;
}

listview& listview::set_redraw(bool doRedraw)
{
	SendMessage(_hWnd, WM_SETREDRAW,
		static_cast<WPARAM>(static_cast<BOOL>(doRedraw)), 0);
	return *this;
}

listview& listview::focus()
{
	SetFocus(_hWnd);
	return *this;
}

listview& listview::set_view(view viewType)
{
	ListView_SetView(_hWnd, static_cast<DWORD>(viewType));
	return *this;
}

listview& listview::icon_push(int iconId)
{
	HIMAGELIST hImg = _proceed_imagelist();
	HICON icon = static_cast<HICON>(LoadImage(
		reinterpret_cast<HINSTANCE>(GetWindowLongPtr(_hWnd, GWLP_HINSTANCE)),
		MAKEINTRESOURCE(iconId), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR));
	ImageList_AddIcon(hImg, icon);
	DestroyIcon(icon);
	return *this;
}

listview& listview::icon_push(const wchar_t *fileExtension)
{
	HIMAGELIST hImg = _proceed_imagelist();
	if (hImg) {
		icon expicon; // icon will be released at the end of this scope block
		expicon.get_from_explorer(fileExtension);
		ImageList_AddIcon(hImg, expicon.hicon()); // append a clone of icon handle to imagelist
	}
	return *this; // return the index of the new icon
}

listview& listview::column_add(const wchar_t *caption, int cx)
{
	LVCOLUMN lvc = { 0 };

	lvc.mask = LVCF_TEXT | LVCF_WIDTH;
	lvc.pszText = const_cast<wchar_t*>(caption);
	lvc.cx = cx;

	ListView_InsertColumn(_hWnd, 0xFFFF, &lvc);
	return *this;
}

listview& listview::column_fit(size_t columnIndex)
{
	size_t numCols = column_count();
	int cxUsed = 0;

	for (size_t i = 0; i < numCols; ++i) {
		if (i != columnIndex) {
			LVCOLUMN lvc = { 0 };
			lvc.mask = LVCF_WIDTH;
			ListView_GetColumn(_hWnd, i, &lvc); // retrieve cx of each column, except stretchee
			cxUsed += lvc.cx; // sum up
		}
	}

	RECT rc = { 0 };
	GetClientRect(_hWnd, &rc); // listview client area
	ListView_SetColumnWidth(_hWnd, columnIndex,
		rc.right /*- GetSystemMetrics(SM_CXVSCROLL)*/ - cxUsed); // fit the rest of available space
	return *this;
}

vector<wstring> listview::get_all_text(vector<item> items, size_t columnIndex)
{
	vector<wstring> texts;
	texts.reserve(items.size());
	for (item oneItem : items) {
		texts.emplace_back(oneItem.get_text(columnIndex));
	}
	return texts;
}

HIMAGELIST listview::_proceed_imagelist()
{
	// Imagelist is destroyed automatically:
	// http://www.catch22.net/tuts/sysimgq
	// http://www.autohotkey.com/docs/commands/ListView.htm

	HIMAGELIST hImg = ListView_GetImageList(_hWnd, LVSIL_SMALL); // current imagelist
	if (!hImg) {
		hImg = ImageList_Create(16, 16, ILC_COLOR32, 1, 1); // create a 16x16 imagelist
		if (!hImg) {
			sys::msg_box(GetParent(_hWnd),
				L"Internal error",
				L"listview::_proceedImagelist\nImageList_Create failed.",
				MB_ICONERROR);
			return nullptr;
		}
		ListView_SetImageList(_hWnd, hImg, LVSIL_SMALL); // associate imagelist to listview control
	}
	return hImg; // return handle to current imagelist
}

int listview::_show_context_menu(bool followCursor)
{
	if (!_contextMenu.hmenu()) return -1; // no context menu assigned

	POINT coords = { 0 };
	int itemBelowCursor = -1;

	if (followCursor) { // usually fired with a right-click
		LVHITTESTINFO lvhti = { 0 };
		GetCursorPos(&lvhti.pt); // relative to screen
		ScreenToClient(_hWnd, &lvhti.pt); // now relative to listview
		ListView_HitTest(_hWnd, &lvhti); // item below cursor, if any
		coords = lvhti.pt;
		itemBelowCursor = lvhti.iItem; // -1 if none
		if (itemBelowCursor != -1) { // an item was right-clicked
			if (!sys::has_ctrl() && !sys::has_shift()) {
				if ((ListView_GetItemState(_hWnd, itemBelowCursor, LVIS_SELECTED) & LVIS_SELECTED) == 0) {
					// If right-clicked item isn't currently selected, unselect all and select just it.
					ListView_SetItemState(_hWnd, -1, 0, LVIS_SELECTED);
					ListView_SetItemState(_hWnd, itemBelowCursor, LVIS_SELECTED, LVIS_SELECTED);
				}
				ListView_SetItemState(_hWnd, itemBelowCursor, LVIS_FOCUSED, LVIS_FOCUSED); // focus clicked
			}
		} else if (!sys::has_ctrl() && !sys::has_shift()) {
			ListView_SetItemState(_hWnd, -1, 0, LVIS_SELECTED); // unselect all
		}
		SetFocus(_hWnd); // because a right-click won't set the focus by default
	} else { // usually fired with the context menu keyboard key
		int itemFocused = ListView_GetNextItem(_hWnd, -1, LVNI_FOCUSED);
		if (itemFocused != -1 && ListView_IsItemVisible(_hWnd, itemFocused)) { // item focused and visible
			RECT rcItem = { 0 };
			ListView_GetItemRect(_hWnd, itemFocused, &rcItem, LVIR_BOUNDS); // relative to listview
			coords = { rcItem.left + 16, rcItem.top + (rcItem.bottom - rcItem.top) / 2 };
		} else { // no focused and visible item
			coords = { 6, 10 };
		}
	}

	// The popup menu is created with hDlg as parent, so the menu messages go to it.
	// The lvhti coordinates are relative to listview, and will be mapped into screen-relative.
	_contextMenu.show_at_point(GetParent(_hWnd), coords, _hWnd);
	return itemBelowCursor; // -1 if none
}

LRESULT CALLBACK listview::_proc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp, UINT_PTR idSubclass, DWORD_PTR refData)
{
	switch (msg) {
	case WM_GETDLGCODE:
		if (lp && wp == 'A' && sys::has_ctrl()) { // Ctrl+A to select all items
			reinterpret_cast<MSG*>(lp)->wParam = 0; // prevent propagation, therefore beep
			ListView_SetItemState(hwnd, -1, LVIS_SELECTED, LVIS_SELECTED);
			return DLGC_WANTCHARS;
		} else if (lp && wp == VK_RETURN) { // send Enter key to parent
			NMLVKEYDOWN nmlvkd = { { hwnd, static_cast<WORD>(GetDlgCtrlID(hwnd)), LVN_KEYDOWN }, VK_RETURN, 0 };
			SendMessage(GetAncestor(hwnd, GA_PARENT), WM_NOTIFY, reinterpret_cast<WPARAM>(hwnd),
				reinterpret_cast<LPARAM>(&nmlvkd) );
			reinterpret_cast<MSG*>(lp)->wParam = 0; // prevent propagation, therefore beep
			return DLGC_WANTALLKEYS;
		} else if (lp && wp == VK_APPS) { // context menu keyboard key
			reinterpret_cast<listview*>(refData)->_show_context_menu(false);
		}
		break;
	case WM_RBUTTONDOWN:
		reinterpret_cast<listview*>(refData)->_show_context_menu(true);
		return 0;
	case WM_NCDESTROY:
		RemoveWindowSubclass(hwnd, _proc, idSubclass);
	}
	return DefSubclassProc(hwnd, msg, wp, lp);
}