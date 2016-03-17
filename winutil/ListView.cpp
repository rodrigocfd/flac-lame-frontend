
#include "ListView.h"
#include "Icon.h"
#include "Str.h"
#include "Sys.h"
using std::pair;
using std::vector;
using std::wstring;

ListView::Item::Item(int itemIndex, ListView *pList)
	: index(itemIndex), _list(pList)
{
}

ListView::Item::Item()
	: Item(-1, nullptr)
{
}

void ListView::Item::remove()
{
	ListView_DeleteItem(_list->_hWnd, index);
}

void ListView::Item::swapWith(size_t itemIndex)
{
	Item newItem = _list->items[itemIndex];

	int numCols = _list->columnCount();
	wstring tmpstr;
	for (int c = 0; c < numCols; ++c) { // swap texts of all columns
		tmpstr = getText(c);
		setText(newItem.getText(c), c);
		newItem.setText(tmpstr, c);
	}

	LPARAM oldp = getParam(); // swap LPARAMs
	setParam(newItem.getParam());
	newItem.setParam(oldp);

	int oldi = getIcon(); // swap icons
	setIcon(newItem.getIcon());
	newItem.setIcon(oldi);
}

ListView::Item& ListView::Item::ensureVisible()
{
	if (_list->getView() == View::DETAILS) {
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
		lvii.iItem = index;
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

bool ListView::Item::isVisible() const
{
	return ListView_IsItemVisible(_list->_hWnd, index) == TRUE;
}

ListView::Item& ListView::Item::setSelect(bool select)
{
	ListView_SetItemState(_list->_hWnd, index,
		select ? LVIS_SELECTED : 0, LVIS_SELECTED);
	return *this;
}

bool ListView::Item::isSelected() const
{
	return (ListView_GetItemState(_list->_hWnd,
		index, LVIS_SELECTED) & LVIS_SELECTED) != 0;
}

ListView::Item& ListView::Item::setFocus()
{
	ListView_SetItemState(_list->_hWnd,
		index, LVIS_FOCUSED, LVIS_FOCUSED);
	return *this;
}

bool ListView::Item::isFocused() const
{
	return (ListView_GetItemState(_list->_hWnd,
		index, LVIS_FOCUSED) & LVIS_FOCUSED) != 0;
}

RECT ListView::Item::getRect() const
{
	RECT rc = { 0 };
	ListView_GetItemRect(_list->_hWnd, index, &rc, LVIR_BOUNDS);
	return rc;
}

wstring ListView::Item::getText(size_t columnIndex) const
{
	// http://forums.codeguru.com/showthread.php?351972-Getting-listView-item-text-length
	LVITEM lvi = { 0 };
	lvi.iItem = index;
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

	Str::trimNulls(buf);
	return buf;
}

ListView::Item& ListView::Item::setText(const wchar_t *text, size_t columnIndex)
{
	ListView_SetItemText(_list->_hWnd, index,
		static_cast<int>(columnIndex), const_cast<wchar_t*>(text));
	return *this;
}

ListView::Item& ListView::Item::setText(const wstring& text, size_t columnIndex)
{
	return setText(text.c_str(), columnIndex);
}

LPARAM ListView::Item::getParam() const
{
	LVITEM lvi = { 0 };
	lvi.iItem = index;
	lvi.mask = LVIF_PARAM;

	ListView_GetItem(_list->_hWnd, &lvi);
	return lvi.lParam;
}

ListView::Item& ListView::Item::setParam(LPARAM lp)
{
	LVITEM lvi = { 0 };
	lvi.iItem = index;
	lvi.mask = LVIF_PARAM;
	lvi.lParam = lp;

	ListView_SetItem(_list->_hWnd, &lvi);
	return *this;
}

int ListView::Item::getIcon() const
{
	LVITEM lvi = { 0 };
	lvi.iItem = index;
	lvi.mask = LVIF_IMAGE;

	ListView_GetItem(_list->_hWnd, &lvi);
	return lvi.iImage; // return index of icon within imagelist
}

ListView::Item& ListView::Item::setIcon(int imagelistIconIndex)
{
	LVITEM lvi = { 0 };
	lvi.iItem = index;
	lvi.mask = LVIF_IMAGE;
	lvi.iImage = imagelistIconIndex;

	ListView_SetItem(_list->_hWnd, &lvi);
	return *this;
}


ListView::Collection::Collection(ListView *pList)
	: _list(pList)
{
}

ListView::Item ListView::Collection::operator[](size_t itemIndex)
{
	return Item(static_cast<int>(itemIndex), _list);
}

int ListView::Collection::count() const
{
	return ListView_GetItemCount(_list->_hWnd);
}

ListView::Item ListView::Collection::add(const wchar_t *caption, int imagelistIconIndex, int positionIndex)
{
	LVITEM lvi = { 0 };
	lvi.iItem = (positionIndex == -1 ? 0x0FFFFFFF : positionIndex);
	lvi.mask = LVIF_TEXT | (imagelistIconIndex == -1 ? 0 : LVIF_IMAGE);
	lvi.pszText = const_cast<wchar_t*>(caption);
	lvi.iImage = imagelistIconIndex;

	return Item(ListView_InsertItem(_list->_hWnd, &lvi), _list); // return index of newly inserted item
}

ListView::Item ListView::Collection::add(const wstring& caption, int imagelistIconIndex, int positionIndex)
{
	return add(caption.c_str(), imagelistIconIndex, positionIndex);
}

vector<ListView::Item> ListView::Collection::getAll() const
{
	int totItems = count();

	vector<Item> items; // a big array with all items in list
	items.reserve(totItems);

	for (int i = 0; i < totItems; ++i) {
		items.emplace_back(i, _list);
	}
	return items;
}

void ListView::Collection::removeAll()
{
	ListView_DeleteAllItems(_list->_hWnd);
}

ListView::Item ListView::Collection::find(const wchar_t *caption)
{
	LVFINDINFO lfi = { 0 };
	lfi.flags = LVFI_STRING; // search is case-insensitive
	lfi.psz = caption;

	return Item(ListView_FindItem(_list->_hWnd, -1, &lfi), _list); // returns -1 if not found
}

ListView::Item ListView::Collection::find(const wstring& caption)
{
	return find(caption.c_str());
}

bool ListView::Collection::exists(const wchar_t *caption)
{
	return find(caption).index != -1;
}

bool ListView::Collection::exists(const wstring& caption)
{
	return exists(caption.c_str());
}

int ListView::Collection::countSelected() const
{
	return ListView_GetSelectedCount(_list->_hWnd);
}

void ListView::Collection::select(const vector<size_t>& indexes)
{
	// Select the items whose indexes have been passed in the array.
	for (const size_t& index : indexes) {
		ListView_SetItemState(_list->_hWnd,
			static_cast<int>(index), LVIS_SELECTED, LVIS_SELECTED);
	}
}

void ListView::Collection::selectAll()
{
	ListView_SetItemState(_list->_hWnd, -1, LVIS_SELECTED, LVIS_SELECTED);
}

void ListView::Collection::selectNone()
{
	ListView_SetItemState(_list->_hWnd, -1, 0, LVIS_SELECTED);
}

void ListView::Collection::removeSelected()
{
	_list->setRedraw(false);
	int i = -1;
	while ((i = ListView_GetNextItem(_list->_hWnd, -1, LVNI_SELECTED)) != -1) {
		ListView_DeleteItem(_list->_hWnd, i);
	}
	_list->setRedraw(true);
}

vector<ListView::Item> ListView::Collection::getSelected() const
{
	vector<Item> items;
	items.reserve(countSelected());

	int iBase = -1;
	for (;;) {
		iBase = ListView_GetNextItem(_list->_hWnd, iBase, LVNI_SELECTED);
		if (iBase == -1) break;
		items.emplace_back(iBase, _list);
	}
	return items;
}

ListView::Item ListView::Collection::getFocused() const
{
	return Item(ListView_GetNextItem(_list->_hWnd, -1, LVNI_FOCUSED), _list);
}


ListView::~ListView()
{
	_contextMenu.destroy();
}

ListView::ListView()
	: _hWnd(nullptr), items(this)
{
}

ListView::ListView(HWND hwnd)
	: ListView()
{
	operator=(hwnd);
}

ListView& ListView::operator=(HWND hwnd)
{
	const int IDSUBCLASS = 1;

	if (_hWnd) { // if previously assigned, remove previous subclassing
		RemoveWindowSubclass(_hWnd, _proc, IDSUBCLASS);
	}

	_hWnd = hwnd;
	SetWindowSubclass(_hWnd, _proc, IDSUBCLASS, reinterpret_cast<DWORD_PTR>(this));
	items = Collection(this); // initialize internal object
	_contextMenu.destroy();
	return *this;
}

ListView& ListView::operator=(pair<HWND, int> hWndAndCtrlId)
{
	return operator=(GetDlgItem(hWndAndCtrlId.first, hWndAndCtrlId.second));
}

HWND ListView::hWnd() const
{
	return _hWnd;
}

ListView& ListView::create(HWND hParent, int id, POINT pos, SIZE size, View view)
{
	// For children, WS_BORDER gives old, flat drawing; always use WS_EX_CLIENTEDGE.
	return operator=( CreateWindowEx(WS_EX_CLIENTEDGE, WC_LISTVIEW, nullptr,
		WS_CHILD | WS_VISIBLE | WS_TABSTOP | static_cast<DWORD>(view),
		pos.x, pos.y, size.cx, size.cy,
		hParent, reinterpret_cast<HMENU>(static_cast<UINT_PTR>(id)),
		reinterpret_cast<HINSTANCE>(GetWindowLongPtr(hParent, GWLP_HINSTANCE)),
		nullptr) );
}

ListView& ListView::setContextMenu(int contextMenuId)
{
	_contextMenu.loadResource(contextMenuId, 0,
		reinterpret_cast<HINSTANCE>(GetWindowLongPtr(_hWnd, GWLP_HINSTANCE)));
	return *this;
}

ListView& ListView::setFullRowSelect()
{
	ListView_SetExtendedListViewStyle(_hWnd, LVS_EX_FULLROWSELECT);
	return *this;
}

ListView& ListView::setRedraw(bool doRedraw)
{
	SendMessage(_hWnd, WM_SETREDRAW,
		static_cast<WPARAM>(static_cast<BOOL>(doRedraw)), 0);
	return *this;
}

ListView& ListView::focus()
{
	SetFocus(_hWnd);
	return *this;
}

ListView& ListView::setView(View view)
{
	ListView_SetView(_hWnd, static_cast<DWORD>(view));
	return *this;
}

ListView::View ListView::getView() const
{
	return static_cast<View>(ListView_GetView(_hWnd));
}

ListView& ListView::iconPush(int iconId)
{
	HIMAGELIST hImg = _proceedImagelist();
	HICON icon = static_cast<HICON>(LoadImage(
		reinterpret_cast<HINSTANCE>(GetWindowLongPtr(_hWnd, GWLP_HINSTANCE)),
		MAKEINTRESOURCE(iconId), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR));
	ImageList_AddIcon(hImg, icon);
	DestroyIcon(icon);
	return *this;
}

ListView& ListView::iconPush(const wchar_t *fileExtension)
{
	HIMAGELIST hImg = _proceedImagelist();
	if (hImg) {
		Icon expicon; // icon will be released at the end of this scope block
		expicon.getFromExplorer(fileExtension);
		ImageList_AddIcon(hImg, expicon.hIcon()); // append a clone of icon handle to imagelist
	}
	return *this; // return the index of the new icon
}

int ListView::columnCount() const
{
	return Header_GetItemCount(ListView_GetHeader(_hWnd));
}

ListView& ListView::columnAdd(const wchar_t *caption, int cx)
{
	LVCOLUMN lvc = { 0 };

	lvc.mask = LVCF_TEXT | LVCF_WIDTH;
	lvc.pszText = const_cast<wchar_t*>(caption);
	lvc.cx = cx;

	ListView_InsertColumn(_hWnd, 0xFFFF, &lvc);
	return *this;
}

ListView& ListView::columnFit(size_t columnIndex)
{
	int numCols = columnCount();
	int cxUsed = 0;

	for (int i = 0; i < numCols; ++i) {
		if (i != columnIndex) {
			LVCOLUMN lvc = { 0 };
			lvc.mask = LVCF_WIDTH;
			ListView_GetColumn(_hWnd, i, &lvc); // retrieve cx of each column, except stretchee
			cxUsed += lvc.cx; // sum up
		}
	}

	RECT rc = { 0 };
	GetClientRect(_hWnd, &rc); // listview client area
	ListView_SetColumnWidth(_hWnd, static_cast<int>(columnIndex),
		rc.right /*- GetSystemMetrics(SM_CXVSCROLL)*/ - cxUsed); // fit the rest of available space
	return *this;
}

vector<wstring> ListView::getAllText(vector<Item> items, size_t columnIndex)
{
	vector<wstring> texts;
	texts.reserve(items.size());
	for (Item item : items) {
		texts.emplace_back(item.getText(columnIndex));
	}
	return texts;
}

HIMAGELIST ListView::_proceedImagelist()
{
	// Imagelist is destroyed automatically:
	// http://www.catch22.net/tuts/sysimgq
	// http://www.autohotkey.com/docs/commands/ListView.htm

	HIMAGELIST hImg = ListView_GetImageList(_hWnd, LVSIL_SMALL); // current imagelist
	if (!hImg) {
		hImg = ImageList_Create(16, 16, ILC_COLOR32, 1, 1); // create a 16x16 imagelist
		if (!hImg) {
			Sys::msgBox(GetParent(_hWnd),
				L"Internal error",
				L"ListView::_proceedImagelist\nImageList_Create failed.",
				MB_ICONERROR);
			return nullptr;
		}
		ListView_SetImageList(_hWnd, hImg, LVSIL_SMALL); // associate imagelist to listview control
	}
	return hImg; // return handle to current imagelist
}

int ListView::_showContextMenu(bool followCursor)
{
	if (!_contextMenu.hMenu()) return -1; // no context menu assigned

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
			if (!Sys::hasCtrl() && !Sys::hasShift()) {
				if ((ListView_GetItemState(_hWnd, itemBelowCursor, LVIS_SELECTED) & LVIS_SELECTED) == 0) {
					// If right-clicked item isn't currently selected, unselect all and select just it.
					ListView_SetItemState(_hWnd, -1, 0, LVIS_SELECTED);
					ListView_SetItemState(_hWnd, itemBelowCursor, LVIS_SELECTED, LVIS_SELECTED);
				}
				ListView_SetItemState(_hWnd, itemBelowCursor, LVIS_FOCUSED, LVIS_FOCUSED); // focus clicked
			}
		}
		else if (!Sys::hasCtrl() && !Sys::hasShift()) {
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
	_contextMenu.showAtPoint(GetParent(_hWnd), coords, _hWnd);
	return itemBelowCursor; // -1 if none
}

LRESULT CALLBACK ListView::_proc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp,
	UINT_PTR idSubclass, DWORD_PTR refData)
{
	switch (msg) {
	case WM_GETDLGCODE:
		if (lp && wp == 'A' && Sys::hasCtrl()) { // Ctrl+A to select all items
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
			reinterpret_cast<ListView*>(refData)->_showContextMenu(false);
		}
		break;
	case WM_RBUTTONDOWN:
		reinterpret_cast<ListView*>(refData)->_showContextMenu(true);
		return 0;
	case WM_NCDESTROY:
		RemoveWindowSubclass(hwnd, _proc, idSubclass);
	}
	return DefSubclassProc(hwnd, msg, wp, lp);
}