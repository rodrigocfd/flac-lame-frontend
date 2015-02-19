/*!
 * Often-used controls.
 * Part of OWL - Object Win32 Library.
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/wolf
 */

#include "Controls.h"
using namespace owl;
using std::function;
using std::initializer_list;
using std::vector;
using std::wstring;

Menu& Menu::createMain(HWND owner)
{
	this->destroy();
	_hMenu = CreateMenu(); // to be used as a main window menu
	this->appendItem(L"_DUMMY_", WM_APP-2); // avoids further call to DrawMenuBar(), which would demand HWND again
	SetMenu(owner, _hMenu);
	return *this;
}

Menu& Menu::createPopup()
{
	this->destroy();
	_hMenu = CreatePopupMenu(); // to be used as a popup menu
	return *this;
}

Menu& Menu::appendSeparator()
{
	this->_checkDummyEntry();
	InsertMenu(_hMenu, -1, MF_BYPOSITION | MF_SEPARATOR, 0, nullptr);
	return *this;
}

Menu& Menu::appendItem(const wchar_t *caption, WORD cmdId)
{
	this->_checkDummyEntry();
	InsertMenu(_hMenu, -1, MF_BYPOSITION | MF_STRING, cmdId, caption);
	return *this;
}

Menu& Menu::enableItem(initializer_list<WORD> cmdIds, bool doEnable)
{
	for (const WORD& cmd : cmdIds) {
		EnableMenuItem(_hMenu, cmd, MF_BYCOMMAND | ((doEnable) ? MF_ENABLED : MF_GRAYED));
	}
	return *this;
}

Menu Menu::appendSubmenu(const wchar_t *caption)
{
	this->_checkDummyEntry();
	
	Menu sub;
	sub.createPopup();
	AppendMenu(_hMenu, MF_STRING | MF_POPUP, (UINT_PTR)sub.hMenu(), caption);
	return sub; // return new submenu, so it can be edited
}

void Menu::_checkDummyEntry()
{
	if (size() == 1 && GetMenuItemID(_hMenu, 0) == WM_APP-2) {
		DeleteMenu(_hMenu, 0, MF_BYPOSITION); // delete dummy, if any
	}
}


Resizer& Resizer::add(initializer_list<HWND> hChildren, Do modeHorz, Do modeVert)
{
	_ctrls.reserve(_ctrls.size() + hChildren.size());
	for (const HWND& hChild : hChildren) {
		this->_addOne(hChild, modeHorz, modeVert);
	}
	return *this;
}

Resizer& Resizer::add(initializer_list<int> ctrlIds, HWND hParent, Do modeHorz, Do modeVert)
{
	_ctrls.reserve(_ctrls.size() + ctrlIds.size());
	for (const int& ctrlId : ctrlIds) {
		this->_addOne(GetDlgItem(hParent, ctrlId), modeHorz, modeVert);
	}
	return *this;
}

void Resizer::_addOne(Window ctrl, Do modeHorz, Do modeVert)
{
	Window parent = ctrl.getParent();

	if (!_ctrls.size()) { // first call to _addOne()
		RECT rcP = parent.getClientRect();
		_szOrig.cx = rcP.right;
		_szOrig.cy = rcP.bottom; // save original size of parent
		SetWindowSubclass(parent.hWnd(), _Proc, 1,
			reinterpret_cast<DWORD_PTR>(this)); // subclass parent, we'll manage WM_SIZE
	}

	_ctrls.push_back({ ctrl, ctrl.getWindowRect(), modeHorz, modeVert });
	parent.screenToClient(reinterpret_cast<POINT*>(&_ctrls.back().rcOrig));
	parent.screenToClient(reinterpret_cast<POINT*>(&_ctrls.back().rcOrig.right)); // client coordinates relative to parent
}

LRESULT CALLBACK Resizer::_Proc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp, UINT_PTR idSubclass, DWORD_PTR refData)
{
	switch (msg)
	{
	case WM_SIZE:
		{
			Resizer *pThis = reinterpret_cast<Resizer*>(refData);
			int state = static_cast<int>(wp);
			int cx = LOWORD(lp);
			int cy = HIWORD(lp);
			if (pThis->_ctrls.size() && state != SIZE_MINIMIZED) { // only if created() was called; if minimized, no need to resize
				HDWP hdwp = BeginDeferWindowPos(static_cast<int>(pThis->_ctrls.size()));
				for (const _Ctrl& ctrl : pThis->_ctrls) {
					UINT uFlags = SWP_NOZORDER;
					if (ctrl.modeHorz == Do::REPOS && ctrl.modeVert == Do::REPOS) { // reposition both vert & horz
						uFlags |= SWP_NOSIZE;
					} else if (ctrl.modeHorz == Do::RESIZE && ctrl.modeVert == Do::RESIZE) { // resize both vert & horz
						uFlags |= SWP_NOMOVE;
					}

					DeferWindowPos(hdwp, ctrl.wnd.hWnd(), nullptr,
						ctrl.modeHorz == Do::REPOS ?
							cx - pThis->_szOrig.cx + ctrl.rcOrig.left :
							ctrl.rcOrig.left, // keep original pos
						ctrl.modeVert == Do::REPOS ?
							cy - pThis->_szOrig.cy + ctrl.rcOrig.top :
							ctrl.rcOrig.top, // keep original pos
						ctrl.modeHorz == Do::RESIZE ?
							cx - pThis->_szOrig.cx + ctrl.rcOrig.right - ctrl.rcOrig.left :
							ctrl.rcOrig.right - ctrl.rcOrig.left, // keep original width
						ctrl.modeVert == Do::RESIZE ?
							cy - pThis->_szOrig.cy + ctrl.rcOrig.bottom - ctrl.rcOrig.top :
							ctrl.rcOrig.bottom - ctrl.rcOrig.top, // keep original height
						uFlags);
				}
				EndDeferWindowPos(hdwp);
				if (pThis->_afterResize) pThis->_afterResize(); // invoke user callback, if any
			}
		}
		break;
	case WM_NCDESTROY:
		RemoveWindowSubclass(hWnd, _Proc, idSubclass); // http://blogs.msdn.com/b/oldnewthing/archive/2003/11/11/55653.aspx
	}
	return DefSubclassProc(hWnd, msg, wp, lp);
}


TextBox& TextBox::operator=(HWND hwnd)
{
	const int IDSUBCLASS = 1;

	if (this->hWnd()) { // remove any previous subclassing of us, will be reassigned
		RemoveWindowSubclass(this->hWnd(), _Proc, IDSUBCLASS);
	}

	static_cast<Window*>(this)->operator=(hwnd);
	_notifyKeyUp = 0;
	SetWindowSubclass(this->hWnd(), _Proc, IDSUBCLASS, reinterpret_cast<DWORD_PTR>(this));
	return *this;
}

TextBox& TextBox::create(WindowPopup *parent, int id, POINT pos, int cx, UINT extraStyles)
{
	this->operator=( CreateWindowEx(WS_EX_CLIENTEDGE, WC_EDIT, nullptr,
		WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL | extraStyles,
		pos.x, pos.y, cx, 21,
		parent->hWnd(), reinterpret_cast<HMENU>(id), parent->getInstance(), nullptr) );
	return *this;
}

TextBox& TextBox::setFont(const Font& font)
{
	// Call this method within WM_SHOWWINDOW processing.
	// If called during WM_INITDIALOG, it will be undone during default processing.

	_font.cloneFrom(font); // since it's cloned, user font may be safely deleted
	_font.apply(hWnd());
	return *this;
}

void TextBox::selGet(int *start, int *length)
{
	int p1 = 0;
	this->sendMessage(EM_GETSEL, reinterpret_cast<WPARAM>(start), reinterpret_cast<LPARAM>(&p1));
	*length = p1 - (*start);
}

LRESULT CALLBACK TextBox::_Proc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp, UINT_PTR idSubclass, DWORD_PTR refData)
{
	TextBox *pSelf = reinterpret_cast<TextBox*>(refData);

	switch (msg)
	{
	case WM_KEYDOWN:
		// http://www.williamwilling.com/blog/?p=28
		switch (LOWORD(wp))
		{
		case VK_ESCAPE: // ESC
			SendMessage(GetAncestor(hWnd, GA_PARENT), WM_COMMAND, IDCANCEL, reinterpret_cast<LPARAM>(hWnd));
			return 0;
		}
		break;
	case WM_GETDLGCODE:
		if (lp && wp == 'A' && System::HasCtrl()) { // Ctrl+A to select all text
			reinterpret_cast<MSG*>(lp)->wParam = 0; // prevent propagation, therefore beep
			SendMessage(hWnd, EM_SETSEL, 0, -1);
			return DLGC_WANTCHARS;
		}
		break;
	case WM_KEYUP:
		if (pSelf->_notifyKeyUp) pSelf->getParent().sendMessage(pSelf->_notifyKeyUp, wp, lp);
		break;
	case WM_NCDESTROY:
		RemoveWindowSubclass(hWnd, _Proc, idSubclass);
	}
	return DefSubclassProc(hWnd, msg, wp, lp);
}


Combo& Combo::create(WindowPopup *parent, int id, POINT pos, int cx)
{
	this->operator=( CreateWindowEx(0, WC_COMBOBOX, nullptr,
		WS_CHILD | WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWNLIST | CBS_SORT,
		pos.x, pos.y, cx, 0,
		parent->hWnd(), reinterpret_cast<HMENU>(id), parent->getInstance(), nullptr) );
	return *this;
}

Combo& Combo::itemAdd(initializer_list<const wchar_t*> arrStr)
{
	for (const wchar_t *s : arrStr) {
		this->sendMessage(CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(s));
	}
	return *this;
}

wchar_t* Combo::itemGetText(int i, wchar_t *pBuf, int szBuf) const
{
	int len = static_cast<int>(this->sendMessage(CB_GETLBTEXTLEN, i, 0)) + 1;
	if (szBuf < len) {
		*pBuf = L'\0'; // buffer is too small
	} else {
		this->sendMessage(CB_GETLBTEXT, i, reinterpret_cast<LPARAM>(pBuf));
	}
	return pBuf;
}

wstring& Combo::itemGetText(int i, wstring& buf) const
{
	buf.clear();
	buf.resize(static_cast<int>(this->sendMessage(CB_GETLBTEXTLEN, i, 0)) + 1);
	this->sendMessage(CB_GETLBTEXT, i, reinterpret_cast<LPARAM>(&buf[0]));
	buf.resize(buf.size() - 1); // remove unnecessary terminating null
	return buf;
}


ListBox& ListBox::create(WindowPopup *parent, int id, POINT pos, SIZE size)
{
	this->operator=( CreateWindowEx(WS_EX_CLIENTEDGE, WC_LISTBOX, nullptr,
		WS_CHILD | WS_VISIBLE | WS_TABSTOP,
		pos.x, pos.y, size.cx, size.cy,
		parent->hWnd(), reinterpret_cast<HMENU>(id), parent->getInstance(), nullptr) );
	return *this;
}

ListBox& ListBox::itemAdd(initializer_list<const wchar_t*> arrStr)
{
	for (const wchar_t *s : arrStr) {
		this->sendMessage(LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(s));
	}
	return *this;
}

int ListBox::itemCountSelected() const
{
	int cou = static_cast<int>(this->sendMessage(LB_GETSELCOUNT, 0, 0));
	if (cou == LB_ERR) { // we have a single-selection listbox, zero or one items can be selected
		return (this->sendMessage(LB_GETCURSEL, 0, 0) == LB_ERR) ? 0 : 1;
	}
	return cou;
}

int ListBox::itemGetSelected(vector<int> *indexesBuf) const
{
	if (indexesBuf) {
		indexesBuf->clear();
		indexesBuf->resize(this->itemCountSelected());
		LRESULT gsi = this->sendMessage(LB_GETSELITEMS, static_cast<WPARAM>(indexesBuf->size()),
			reinterpret_cast<LPARAM>(&(*indexesBuf)[0]) );
		if (gsi == LB_ERR) {
			if (indexesBuf->size() > 0) { // a single-selection listbox
				(*indexesBuf)[0] = static_cast<int>(this->sendMessage(LB_GETCURSEL, 0, 0));
			}
		}
		return indexesBuf->size() > 0 ? (*indexesBuf)[0] : -1;
	}
	return static_cast<int>(this->sendMessage(LB_GETCURSEL, 0, 0)); // will work for single-selection listbox only
}

wchar_t* ListBox::itemGetText(int i, wchar_t *pBuf, int szBuf) const
{
	int len = static_cast<int>(this->sendMessage(LB_GETTEXTLEN, i, 0)) + 1;
	if (szBuf < len) {
		*pBuf = 0; // buffer is too small
	} else {
		this->sendMessage(LB_GETTEXT, i, reinterpret_cast<LPARAM>(pBuf));
	}
	return pBuf;
}

wstring& ListBox::itemGetText(int i, wstring& buf) const
{
	buf.clear();
	buf.resize(static_cast<int>(this->sendMessage(LB_GETTEXTLEN, i, 0)) + 1);
	this->sendMessage(LB_GETTEXT, i, reinterpret_cast<LPARAM>(&buf[0]));
	buf.resize(buf.size() - 1); // remove unnecessary terminating null
	return buf;
}


Radio& Radio::create(WindowPopup *parent, int id, const wchar_t *caption, bool beginGroup, POINT pos, SIZE size)
{
	this->operator=( CreateWindowEx(0, WC_BUTTON, caption,
		WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTORADIOBUTTON | (beginGroup ? WS_GROUP : 0),
		pos.x, pos.y, size.cx, size.cy,
		parent->hWnd(), reinterpret_cast<HMENU>(id), parent->getInstance(), nullptr) );
	return *this;
}

void Radio::setCheck(bool checked, Radio::EmulateClick emulateClick)
{
	this->sendMessage(BM_SETCHECK, checked ? BST_CHECKED : BST_UNCHECKED, 0);
	if (emulateClick == EmulateClick::EMULATE)
		this->getParent().sendMessage(WM_COMMAND,
			MAKEWPARAM(GetDlgCtrlID(this->hWnd()), 0),
			reinterpret_cast<LPARAM>(this->hWnd()) );
}


CheckBox& CheckBox::create(WindowPopup *parent, int id, const wchar_t *caption, POINT pos, SIZE size)
{
	this->operator=( CreateWindowEx(0, WC_BUTTON, caption,
		WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX,
		pos.x, pos.y, size.cx, size.cy,
		parent->hWnd(), reinterpret_cast<HMENU>(id), parent->getInstance(), nullptr) );
	return *this;
}


ProgressBar& ProgressBar::create(WindowPopup *parent, int id, POINT pos, SIZE size)
{
	this->operator=( CreateWindowEx(0, PROGRESS_CLASS, nullptr,
		WS_CHILD | WS_VISIBLE,
		pos.x, pos.y, size.cx, size.cy,
		parent->hWnd(), reinterpret_cast<HMENU>(id), parent->getInstance(), nullptr) );
	return *this;
}

ProgressBar& ProgressBar::animateMarquee(bool animate)
{
	if (animate) {
		SetWindowLongPtr(this->hWnd(), GWL_STYLE, // set this on resource editor won't work
			GetWindowLongPtr(this->hWnd(), GWL_STYLE) | PBS_MARQUEE);
	}
	
	this->sendMessage(PBM_SETMARQUEE, static_cast<WPARAM>(animate), 0);
	
	// http://stackoverflow.com/questions/23686724/how-to-reset-marquee-progress-bar
	if (!animate) {
		SetWindowLongPtr(this->hWnd(), GWL_STYLE,
			GetWindowLongPtr(this->hWnd(), GWL_STYLE) & ~PBS_MARQUEE);
	}
	
	return *this;
}


StatusBar& StatusBar::create(HWND hOwner, int numPartsItWillHave)
{
	// The owner is considered resizable if it has the maximize button.
	bool isStretch = (GetWindowLongPtr(hOwner, GWL_STYLE) & WS_MAXIMIZEBOX) != 0;

	_sb = CreateWindowEx(0, STATUSCLASSNAME, nullptr,
		(isStretch ? SBARS_SIZEGRIP : 0) | WS_CHILD | WS_VISIBLE,
		0, 0, 0, 0, hOwner, nullptr,
		reinterpret_cast<HINSTANCE>(GetWindowLongPtr(hOwner, GWLP_HINSTANCE)), nullptr);

	_parts.reserve(numPartsItWillHave);
	_rightEdges.resize(numPartsItWillHave); // only used in _putParts(); allocated through object life for performance
	return *this;
}

StatusBar& StatusBar::addFixedPart(BYTE sizePixels)
{
	_parts.push_back({ sizePixels, 0 });

	if (_parts.size() == _rightEdges.size()) { // last part inserted
		this->_putParts(_sb.getParent().getClientRect().right);
	}
	return *this;
}

StatusBar& StatusBar::addResizablePart(float resizeWeight)
{
	// How resizeWeight works:
	// Suppose you have 3 parts, respectively with weights of 1, 1 and 2.
	// If available client area is 400px, respective part widths will be 100, 100 and 200px.
	// Zero weight means a fixed-width part, which therefore should have sizePixels set (otherwise zero).

	_parts.push_back({ 0, resizeWeight });

	if (_parts.size() == _rightEdges.size()) { // last part inserted
		this->_putParts(_sb.getParent().getClientRect().right);
	}
	return *this;
}

StatusBar& StatusBar::setText(const wchar_t *text, int iPart)
{
	_sb.sendMessage(SB_SETTEXT, MAKEWPARAM(MAKEWORD(iPart, 0), 0), reinterpret_cast<LPARAM>(text));
	return *this;
}

wstring StatusBar::getText(int iPart) const
{
	wstring ret(LOWORD(_sb.sendMessage(SB_GETTEXTLENGTH, iPart, 0)) + 1, L'\0');
	_sb.sendMessage(SB_GETTEXT, iPart, reinterpret_cast<LPARAM>(&ret[0]));
	ret.resize(ret.size() - 1); // remove unnecessary terminating null
	return ret;
}

void StatusBar::_putParts(int cx)
{
	_sb.sendMessage(WM_SIZE, 0, 0); // tell statusbar to fit parent

	// Find the space to be divided among variable-width parts,
	// and total weight of variable-width parts.
	float totalWeight = 0;
	int   cxVariable = cx, cxTotal = cx;

	for (const _Part& part : _parts) {
		if (!part.resizeWeight) { // fixed-width?
			cxVariable -= part.sizePixels;
		} else {
			totalWeight += part.resizeWeight;
		}
	}

	// Fill right edges array with the right edge of each part.
	for (size_t i = _parts.size(); i-- > 0; ) {
		_rightEdges[i] = cxTotal;
		cxTotal -= (!_parts[i].resizeWeight) ? // fixed-width?
			_parts[i].sizePixels :
			static_cast<int>( (cxVariable / totalWeight) * _parts[i].resizeWeight );
	}

	_sb.sendMessage(SB_SETPARTS, _rightEdges.size(), reinterpret_cast<LPARAM>(&_rightEdges[0]));
}


void ListView::Item::swapWith(int index)
{
	Item newItem = this->_list->items[index];

	int numCols = this->_list->columnCount();
	wstring tmpstr;
	for (int c = 0; c < numCols; ++c) { // swap texts of all columns
		tmpstr = this->getText(c);
		this->setText(newItem.getText(c), c);
		newItem.setText(tmpstr, c);
	}

	LPARAM oldp = this->getParam(); // swap LPARAMs
	this->setParam(newItem.getParam());
	newItem.setParam(oldp);

	int oldi = this->getIcon(); // swap icons
	this->setIcon(newItem.getIcon());
	newItem.setIcon(oldi);
}

ListView::Item& ListView::Item::ensureVisible()
{
	if (_list->getView() == View::VW_DETAILS) {
		// In details view, ListView_EnsureVisible() won't center the item vertically.
		// This new implementation has this behavior.
		RECT rc = _list->getClientRect();
		int cyList = rc.bottom; // total height of list

		SecureZeroMemory(&rc, sizeof(rc));
		LVITEMINDEX lvii = { 0 };
		lvii.iItem = ListView_GetTopIndex(_list->hWnd()); // 1st visible item
		ListView_GetItemIndexRect(_list->hWnd(), &lvii, 0, LVIR_BOUNDS, &rc);
		int cyItem = rc.bottom - rc.top; // height of a single item
		int xTop = rc.top; // topmost X of 1st visible item

		SecureZeroMemory(&rc, sizeof(rc));
		SecureZeroMemory(&lvii, sizeof(lvii));
		lvii.iItem = this->i;
		ListView_GetItemIndexRect(_list->hWnd(), &lvii, 0, LVIR_BOUNDS, &rc);
		int xUs = rc.top; // our current X

		if (xUs < xTop || xUs > xTop + cyList) { // if we're not visible
			ListView_Scroll(_list->hWnd(), 0, xUs - xTop - cyList / 2 + cyItem * 2);
		}
	} else {
		ListView_EnsureVisible(_list->hWnd(), this->i, FALSE);
	}
	return *this;
}

wstring& ListView::Item::getText(wstring& buf, int iCol) const
{
	// http://forums.codeguru.com/showthread.php?351972-Getting-listView-item-text-length
	LVITEM lvi = { 0 };
	lvi.iItem = this->i;
	lvi.iSubItem = iCol;

	// Notice that, since strings' size always increase, if the buffer
	// was previously allocated with a value bigger than our 1st step,
	// this will speed up the size checks.

	buf.clear();
	int baseBufLen = 0;
	int charsWrittenWithoutNull = 0;
	do {
		baseBufLen += 64; // buffer increasing step, arbitrary!
		buf.resize(baseBufLen);
		lvi.cchTextMax = static_cast<int>(buf.size());
		lvi.pszText = &buf[0];
		charsWrittenWithoutNull = static_cast<int>(_list->sendMessage(LVM_GETITEMTEXT, this->i,
			reinterpret_cast<LPARAM>(&lvi) ));
	} while (charsWrittenWithoutNull == buf.size() - 1); // to break, must have at least 1 char gap

	TrimNulls(buf);
	return buf;
}

LPARAM ListView::Item::getParam() const
{
	LVITEM lvi = { 0 };
	lvi.iItem = this->i;
	lvi.mask = LVIF_PARAM;

	ListView_GetItem(_list->hWnd(), &lvi);
	return lvi.lParam;
}

ListView::Item& ListView::Item::setParam(LPARAM lp)
{
	LVITEM lvi = { 0 };
	lvi.iItem = this->i;
	lvi.mask = LVIF_PARAM;
	lvi.lParam = lp;

	ListView_SetItem(_list->hWnd(), &lvi);
	return *this;
}

int ListView::Item::getIcon() const
{
	LVITEM lvi = { 0 };
	lvi.iItem = this->i;
	lvi.mask = LVIF_IMAGE;

	ListView_GetItem(_list->hWnd(), &lvi);
	return lvi.iImage; // return index of icon within imagelist
}

ListView::Item& ListView::Item::setIcon(int iconIdx)
{
	LVITEM lvi = { 0 };
	lvi.iItem = this->i;
	lvi.mask = LVIF_IMAGE;
	lvi.iImage = iconIdx; // index of icon within imagelist

	ListView_SetItem(_list->hWnd(), &lvi);
	return *this;
}


ListView::Item ListView::ItemsProxy::add(const wchar_t *caption, int iconIdx, int at)
{
	LVITEM lvi = { 0 };
	lvi.iItem = (at == -1 ? 0x0FFFFFFF : at);
	lvi.mask = LVIF_TEXT | (iconIdx == -1 ? 0 : LVIF_IMAGE);
	lvi.pszText = const_cast<wchar_t*>(caption);
	lvi.iImage = iconIdx; // index of icon within imagelist

	return Item(ListView_InsertItem(_list->hWnd(), &lvi), _list); // return index of newly inserted item
}

vector<ListView::Item> ListView::ItemsProxy::getAll() const
{
    int totItems = this->count();

    vector<Item> items; // a big array with all items in list
    items.reserve(totItems);

    for (int i = 0; i < totItems; ++i) {
        items.emplace_back(i, this->_list);
	}
	return items;
}

ListView::Item ListView::ItemsProxy::find(const wchar_t *caption)
{
	LVFINDINFO lfi = { 0 };
	lfi.flags = LVFI_STRING; // search is case-insensitive
	lfi.psz = caption;

	return Item(ListView_FindItem(_list->hWnd(), -1, &lfi), _list); // returns -1 if not found
}

void ListView::ItemsProxy::select(const vector<int>& idx)
{
	// Select the items whose indexes have been passed in the array.
	for (const int& index : idx) {
		ListView_SetItemState(_list->hWnd(), index, LVIS_SELECTED, LVIS_SELECTED);
	}
}

void ListView::ItemsProxy::removeSelected()
{
	_list->setRedraw(false);
	int i = -1;
	while ((i = ListView_GetNextItem(_list->hWnd(), -1, LVNI_SELECTED)) != -1) {
		ListView_DeleteItem(_list->hWnd(), i);
	}
	_list->setRedraw(true);
}

vector<ListView::Item> ListView::ItemsProxy::getSelected() const
{
	vector<Item> items;
	items.reserve(this->countSelected());

	int iBase = -1;
	for (;;) {
		iBase = ListView_GetNextItem(_list->hWnd(), iBase, LVNI_SELECTED);
		if (iBase == -1) break;
		items.emplace_back(iBase, this->_list);
	}
	return items;
}


ListView& ListView::operator=(HWND hwnd)
{
	const int IDSUBCLASS = 1;

	if (this->hWnd()) { // if previously assigned, remove previous subclassing
		RemoveWindowSubclass(this->hWnd(), _Proc, IDSUBCLASS);
	}

	static_cast<Window*>(this)->operator=(hwnd);
	SetWindowSubclass(this->hWnd(), _Proc, IDSUBCLASS, reinterpret_cast<DWORD_PTR>(this));
	items = ItemsProxy(this); // initialize internal object
	_ctxMenuId = 0; // ID of context popup menu
	return *this;
}

ListView& ListView::create(WindowPopup *parent, int id, POINT pos, SIZE size)
{
	this->operator=( CreateWindowEx(WS_EX_CLIENTEDGE, WC_LISTVIEW, nullptr,
		WS_CHILD | WS_VISIBLE | WS_TABSTOP | LVS_REPORT,
		pos.x, pos.y, size.cx, size.cy,
		parent->hWnd(), reinterpret_cast<HMENU>(id), parent->getInstance(), nullptr) );
	return *this;
}

ListView& ListView::iconPush(int iconId)
{
	HIMAGELIST hImg = this->_proceedImageList();
	HICON icon = static_cast<HICON>(LoadImage(this->getInstance(),
		MAKEINTRESOURCE(iconId), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR));
	ImageList_AddIcon(hImg, icon);
	DestroyIcon(icon);
	return *this;
}

ListView& ListView::iconPush(const wchar_t *fileExtension)
{
	HIMAGELIST hImg = this->_proceedImageList();
	if (hImg) {
		Icon expicon; // icon will be released at the end of this scope block
		expicon.getFromExplorer(fileExtension);
		ImageList_AddIcon(hImg, expicon.hIcon()); // append a clone of icon handle to imagelist
	}
	return *this; // return the index of the new icon
}

ListView& ListView::columnAdd(const wchar_t *caption, int cx)
{
	LVCOLUMN lvc = { 0 };

	lvc.mask = LVCF_TEXT | LVCF_WIDTH;
	lvc.pszText = const_cast<wchar_t*>(caption);
	lvc.cx = cx;

	ListView_InsertColumn(this->hWnd(), 0xFFFF, &lvc);
	return *this;
}

ListView& ListView::columnFit(int iCol)
{
	int numCols = this->columnCount();
	int cxUsed = 0;

	for (int i = 0; i < numCols; ++i) {
		if (i != iCol) {
			LVCOLUMN lvc = { 0 };
			lvc.mask = LVCF_WIDTH;
			ListView_GetColumn(this->hWnd(), i, &lvc); // retrieve cx of each column, except stretchee
			cxUsed += lvc.cx; // sum up
		}
	}

	RECT rc = this->getClientRect(); // listview client area
	ListView_SetColumnWidth(this->hWnd(), iCol,
		rc.right /*- GetSystemMetrics(SM_CXVSCROLL)*/ - cxUsed); // fit the rest of available space
	return *this;
}

HIMAGELIST ListView::_proceedImageList()
{
	// Imagelist is destroyed automatically:
	// http://www.catch22.net/tuts/sysimgq
	// http://www.autohotkey.com/docs/commands/ListView.htm

	HIMAGELIST hImg = ListView_GetImageList(this->hWnd(), LVSIL_SMALL); // current imagelist
	if (!hImg) {
		hImg = ImageList_Create(16, 16, ILC_COLOR32, 1, 1); // create a 16x16 imagelist
		if (!hImg) {
			OutputDebugString(L"ERROR: ImageList_Create failed.\n");
			return nullptr;
		}
		ListView_SetImageList(this->hWnd(), hImg, LVSIL_SMALL); // associate imagelist to listview control
	}
	return hImg; // return handle to current imagelist
}

int ListView::_showCtxMenu(bool followCursor)
{
	if (!_ctxMenuId) return -1; // no context menu assigned via setContextMenu()

	POINT coords = { 0 };
	int itemBelowCursor = -1;

	if (followCursor) { // usually fired with a right-click
		LVHITTESTINFO lvhti = { 0 };
		GetCursorPos(&lvhti.pt); // relative to screen
		this->screenToClient(&lvhti.pt); // now relative to listview
		ListView_HitTest(this->hWnd(), &lvhti); // item below cursor, if any
		coords = lvhti.pt;
		itemBelowCursor = lvhti.iItem; // -1 if none
		if (itemBelowCursor != -1) { // an item was right-clicked
			if (!System::HasCtrl() && !System::HasShift()) {
				if ((ListView_GetItemState(this->hWnd(), itemBelowCursor, LVIS_SELECTED) & LVIS_SELECTED) == 0) {
					// If right-clicked item isn't currently selected, unselect all and select just it.
					ListView_SetItemState(this->hWnd(), -1, 0, LVIS_SELECTED);
					ListView_SetItemState(this->hWnd(), itemBelowCursor, LVIS_SELECTED, LVIS_SELECTED);
				}
				ListView_SetItemState(this->hWnd(), itemBelowCursor, LVIS_FOCUSED, LVIS_FOCUSED); // focus clicked
			}
		} else { // no item was right-clicked
			if (!System::HasCtrl() && !System::HasShift())
				ListView_SetItemState(this->hWnd(), -1, 0, LVIS_SELECTED); // unselect all
		}
		this->setFocus(); // because a right-click won't set the focus by default
	} else { // usually fired with the context menu keyboard key
		int itemFocused = ListView_GetNextItem(this->hWnd(), -1, LVNI_FOCUSED);
		if (itemFocused != -1 && ListView_IsItemVisible(this->hWnd(), itemFocused)) { // item focused and visible
			RECT rcItem = { 0 };
			ListView_GetItemRect(this->hWnd(), itemFocused, &rcItem, LVIR_BOUNDS); // relative to listview
			coords = { rcItem.left + 16, rcItem.top + (rcItem.bottom - rcItem.top) / 2 };
		} else { // no focused and visible item
			coords = { 6, 10 };
		}
	}

	// The popup menu is created with hDlg as parent, so the menu messages go to it.
	// The lvhti coordinates are relative to listview, and will be mapped into screen-relative.
	System::PopMenu(this->getParent().hWnd(), _ctxMenuId, coords.x, coords.y, this->hWnd());
	return itemBelowCursor; // -1 if none
}

LRESULT CALLBACK ListView::_Proc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp, UINT_PTR idSubclass, DWORD_PTR refData)
{
	switch (msg)
	{
	case WM_GETDLGCODE:
		if (lp && wp == 'A' && System::HasCtrl()) { // Ctrl+A to select all items
			reinterpret_cast<MSG*>(lp)->wParam = 0; // prevent propagation, therefore beep
			ListView_SetItemState(hWnd, -1, LVIS_SELECTED, LVIS_SELECTED);
			return DLGC_WANTCHARS;
		} else if (lp && wp == VK_RETURN) { // send Enter key to parent
			NMLVKEYDOWN nmlvkd = { { hWnd, GetDlgCtrlID(hWnd), LVN_KEYDOWN }, VK_RETURN, 0 };
			SendMessage(GetAncestor(hWnd, GA_PARENT), WM_NOTIFY, reinterpret_cast<WPARAM>(hWnd),
				reinterpret_cast<LPARAM>(&nmlvkd) );
			reinterpret_cast<MSG*>(lp)->wParam = 0; // prevent propagation, therefore beep
			return DLGC_WANTALLKEYS;
		} else if (lp && wp == VK_APPS) { // context menu keyboard key
			reinterpret_cast<ListView*>(refData)->_showCtxMenu(false);
		}
		break;
	case WM_RBUTTONDOWN:
		reinterpret_cast<ListView*>(refData)->_showCtxMenu(true);
		return 0;
	case WM_NCDESTROY:
		RemoveWindowSubclass(hWnd, _Proc, idSubclass);
	}
	return DefSubclassProc(hWnd, msg, wp, lp);
}