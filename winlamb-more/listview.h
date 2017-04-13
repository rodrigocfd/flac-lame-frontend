/**
 * Part of WinLamb - Win32 API Lambda Library - More
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winlamb-more
 */

#pragma once
#include "../winlamb/native_control.h"
#include "../winlamb/subclass.h"
#include "base_styles.h"
#include "image_list.h"
#include "menu.h"

/**
 * base_wnd <-- native_control <-- listview
 */

namespace wl {

// Wrapper to listview control from Common Controls library.
class listview final : public native_control {
public:
	class item final {
	private:
		listview* _list;
	public:
		static const size_t npos = -1;
		size_t index;
		item(size_t itemIndex, listview* pList) : index(itemIndex), _list(pList) { }
		item() : item(item::npos, nullptr) { }

		void remove() const {
			ListView_DeleteItem(this->_list->hwnd(), this->index);
		}

		void swap_with(size_t itemIndex) {
			item newItem = this->_list->items[itemIndex];
			size_t numCols = this->_list->column_count();
			std::wstring tmpstr;
			for (size_t c = 0; c < numCols; ++c) { // swap texts of all columns
				tmpstr = this->get_text(c);
				this->set_text(newItem.get_text(c), c);
				newItem.set_text(tmpstr, c);
			}

			LPARAM oldp = this->get_param(); // swap LPARAMs
			this->set_param(newItem.get_param());
			newItem.set_param(oldp);

			int oldi = this->get_icon_index(); // swap icons
			this->set_icon_index(newItem.get_icon_index());
			newItem.set_icon_index(oldi);
		}

		item& ensure_visible() {
			if (this->_list->get_view() == view::DETAILS) {
				// In details view, ListView_EnsureVisible() won't center the item vertically.
				// This new implementation has this behavior.
				RECT rc = { 0 };
				GetClientRect(this->_list->hwnd(), &rc);
				int cyList = rc.bottom; // total height of list

				SecureZeroMemory(&rc, sizeof(rc));
				LVITEMINDEX lvii = { 0 };
				lvii.iItem = ListView_GetTopIndex(this->_list->hwnd()); // 1st visible item
				ListView_GetItemIndexRect(this->_list->hwnd(), &lvii, 0, LVIR_BOUNDS, &rc);
				int cyItem = rc.bottom - rc.top; // height of a single item
				int xTop = rc.top; // topmost X of 1st visible item

				SecureZeroMemory(&rc, sizeof(rc));
				SecureZeroMemory(&lvii, sizeof(lvii));
				lvii.iItem = static_cast<int>(this->index);
				ListView_GetItemIndexRect(this->_list->hwnd(), &lvii, 0, LVIR_BOUNDS, &rc);
				int xUs = rc.top; // our current X

				if (xUs < xTop || xUs > xTop + cyList) { // if we're not visible
					ListView_Scroll(this->_list->hwnd(), 0, xUs - xTop - cyList / 2 + cyItem * 2);
				}
			} else {
				ListView_EnsureVisible(this->_list->hwnd(), this->index, FALSE);
			}
			return *this;
		}

		bool is_visible() const {
			return ListView_IsItemVisible(this->_list->hwnd(), this->index) == TRUE;
		}

		item& set_select(bool select) {
			ListView_SetItemState(this->_list->hwnd(), this->index,
				select ? LVIS_SELECTED : 0, LVIS_SELECTED);
			return *this;
		}

		bool is_selected() const {
			return (ListView_GetItemState(this->_list->hwnd(),
				this->index, LVIS_SELECTED) & LVIS_SELECTED) != 0;
		}

		item& focus() {
			ListView_SetItemState(this->_list->hwnd(),
				this->index, LVIS_FOCUSED, LVIS_FOCUSED);
			return *this;
		}

		bool is_focused() const {
			return (ListView_GetItemState(this->_list->hwnd(),
				this->index, LVIS_FOCUSED) & LVIS_FOCUSED) != 0;
		}

		RECT get_rect() const {
			RECT rc = { 0 };
			ListView_GetItemRect(this->_list->hwnd(), this->index, &rc, LVIR_BOUNDS);
			return rc;
		}

		std::wstring get_text(size_t columnIndex = 0) const {
			// http://forums.codeguru.com/showthread.php?351972-Getting-listView-item-text-length
			LVITEM lvi = { 0 };
			lvi.iItem = static_cast<int>(this->index);
			lvi.iSubItem = static_cast<int>(columnIndex);

			// Notice that, since strings' size always increase, if the buffer
			// was previously allocated with a value bigger than our 1st step,
			// this will speed up the size checks.

			std::wstring buf(64, L'\0'); // speed-up 1st allocation
			int baseBufLen = 0;
			int charsWrittenWithoutNull = 0;
			do {
				baseBufLen += 64; // buffer increasing step, arbitrary!
				buf.resize(baseBufLen);
				lvi.cchTextMax = baseBufLen;
				lvi.pszText = &buf[0];
				charsWrittenWithoutNull = static_cast<int>(
					SendMessageW(this->_list->hwnd(), LVM_GETITEMTEXT,
						this->index, reinterpret_cast<LPARAM>(&lvi)) );
			} while (charsWrittenWithoutNull == baseBufLen - 1); // to break, must have at least 1 char gap

			buf.resize( lstrlenW(buf.c_str()) ); // str::trim_nulls()
			return buf;
		}

		item& set_text(const wchar_t* text, size_t columnIndex = 0) {
			ListView_SetItemText(this->_list->hwnd(), this->index,
				static_cast<int>(columnIndex), const_cast<wchar_t*>(text));
			return *this;
		}

		item& set_text(const std::wstring& text, size_t columnIndex = 0) {
			return this->set_text(text.c_str(), columnIndex);
		}

		LPARAM get_param() const {
			LVITEM lvi = { 0 };
			lvi.iItem = static_cast<int>(this->index);
			lvi.mask = LVIF_PARAM;

			ListView_GetItem(this->_list->hwnd(), &lvi);
			return lvi.lParam;
		}

		item& set_param(LPARAM lp) {
			LVITEM lvi = { 0 };
			lvi.iItem = static_cast<int>(this->index);
			lvi.mask = LVIF_PARAM;
			lvi.lParam = lp;

			ListView_SetItem(this->_list->hwnd(), &lvi);
			return *this;
		}

		int get_icon_index() const {
			LVITEM lvi = { 0 };
			lvi.iItem = static_cast<int>(this->index);
			lvi.mask = LVIF_IMAGE;

			ListView_GetItem(this->_list->hwnd(), &lvi);
			return lvi.iImage; // return index of icon within imagelist
		}

		item& set_icon_index(int imagelistIconIndex) {
			LVITEM lvi = { 0 };
			lvi.iItem = static_cast<int>(this->index);
			lvi.mask = LVIF_IMAGE;
			lvi.iImage = imagelistIconIndex;

			ListView_SetItem(this->_list->hwnd(), &lvi);
			return *this;
		}
	};

	class collection final {
	private:
		listview* _list;
	public:
		explicit collection(listview* pList) : _list(pList) { }
		item operator[](size_t itemIndex) const { return item(static_cast<int>(itemIndex), this->_list); }
		size_t count() const { return ListView_GetItemCount(this->_list->hwnd()); }

		item add(const wchar_t* caption, int imagelistIconIndex = -1, size_t positionIndex = item::npos) {
			LVITEM lvi = { 0 };
			lvi.iItem = static_cast<int>(positionIndex == -1 ? 0x0FFFFFFF : positionIndex);
			lvi.mask = LVIF_TEXT | (imagelistIconIndex == -1 ? 0 : LVIF_IMAGE);
			lvi.pszText = const_cast<wchar_t*>(caption);
			lvi.iImage = imagelistIconIndex;

			return item(ListView_InsertItem(this->_list->hwnd(), &lvi), this->_list); // return newly inserted item
		}

		item add(const std::wstring& caption, int imagelistIconIndex = -1, size_t positionIndex = item::npos) {
			return this->add(caption.c_str(), imagelistIconIndex, positionIndex);
		}

		std::vector<item> get_all() const {
			size_t totItems = this->count();
			std::vector<item> items; // a big array with all items in list
			items.reserve(totItems);
			for (size_t i = 0; i < totItems; ++i) {
				items.emplace_back(i, this->_list);
			}
			return items;
		}

		void remove_all() {
			ListView_DeleteAllItems(this->_list->hwnd());
		}

		item find(const wchar_t* caption) const {
			LVFINDINFO lfi = { 0 };
			lfi.flags = LVFI_STRING; // search is case-insensitive
			lfi.psz = caption;
			return item(ListView_FindItem(this->_list->hwnd(), -1, &lfi), this->_list); // returns -1 if not found
		}

		item find(const std::wstring& caption) const {
			return this->find(caption.c_str());
		}

		bool exists(const wchar_t* caption) const {
			return this->find(caption).index != item::npos;
		}

		bool exists(const std::wstring& caption) const {
			return this->exists(caption.c_str());
		}

		size_t count_selected() const {
			return ListView_GetSelectedCount(this->_list->hwnd());
		}

		void select(const std::vector<size_t>& indexes) const {
			// Select the items whose indexes have been passed in the array.
			for (const size_t& index : indexes) {
				ListView_SetItemState(this->_list->hwnd(),
					static_cast<int>(index), LVIS_SELECTED, LVIS_SELECTED);
			}
		}

		void select_all() const {
			ListView_SetItemState(this->_list->hwnd(), -1, LVIS_SELECTED, LVIS_SELECTED);
		}

		void select_none() const {
			ListView_SetItemState(this->_list->hwnd(), -1, 0, LVIS_SELECTED);
		}

		void remove_selected() const {
			this->_list->set_redraw(false);
			int i = -1;
			while ((i = ListView_GetNextItem(this->_list->hwnd(), -1, LVNI_SELECTED)) != -1) {
				ListView_DeleteItem(this->_list->hwnd(), i);
			}
			this->_list->set_redraw(true);
		}

		std::vector<item> get_selected() const {
			std::vector<item> items;
			items.reserve(this->count_selected());

			int iBase = -1;
			for (;;) {
				iBase = ListView_GetNextItem(this->_list->hwnd(), iBase, LVNI_SELECTED);
				if (iBase == -1) break;
				items.emplace_back(iBase, this->_list);
			}
			return items;
		}

		std::vector<size_t> get_selected_indexes() const {
			std::vector<size_t> indexes;
			indexes.reserve(this->count_selected());

			int iBase = -1;
			for (;;) {
				iBase = ListView_GetNextItem(this->_list->hwnd(), iBase, LVNI_SELECTED);
				if (iBase == -1) break;
				indexes.emplace_back(iBase);
			}
			return indexes;
		}

		item get_focused() const {
			return item(ListView_GetNextItem(this->_list->hwnd(), -1, LVNI_FOCUSED), this->_list);
		}

		size_t get_focused_index() const {
			int iFoc = ListView_GetNextItem(this->_list->hwnd(), -1, LVNI_FOCUSED);
			return iFoc == -1 ? item::npos : iFoc;
		}
	};

	class styler final : public base::styles<listview> {
	public:
		explicit styler(listview* pList) : styles(pList) { }

		listview& always_show_sel(bool doSet) {
			return this->styles::set_style(doSet, LVS_SHOWSELALWAYS);
		}

		listview& edit_labels(bool doSet) {
			return this->styles::set_style(doSet, LVS_EDITLABELS);
		}

		listview& multiple_sel(bool doSet) {
			return this->styles::set_style(!doSet, LVS_SINGLESEL);
		}

		listview& show_headers(bool doSet) {
			return this->styles::set_style(doSet, LVS_NOCOLUMNHEADER);
		}

		listview& checkboxes(bool doSet) {
			ListView_SetExtendedListViewStyleEx(this->target().hwnd(), LVS_EX_CHECKBOXES,
				doSet ? LVS_EX_CHECKBOXES : 0);
			return this->target();
		}

		listview& double_buffer(bool doSet) {
			ListView_SetExtendedListViewStyleEx(this->target().hwnd(), LVS_EX_DOUBLEBUFFER,
				doSet ? LVS_EX_DOUBLEBUFFER : 0);
			return this->target();
		}

		listview& full_row_select(bool doSet) {
			ListView_SetExtendedListViewStyleEx(this->target().hwnd(), LVS_EX_FULLROWSELECT,
				doSet ? LVS_EX_FULLROWSELECT : 0);
			return this->target();
		}

		listview& grid_lines(bool doSet) {
			ListView_SetExtendedListViewStyleEx(this->target().hwnd(), LVS_EX_GRIDLINES,
				doSet ? LVS_EX_GRIDLINES : 0);
			return this->target();
		}

		listview& reorder_header(bool doSet) {
			ListView_SetExtendedListViewStyleEx(this->target().hwnd(), LVS_EX_HEADERDRAGDROP,
				doSet ? LVS_EX_HEADERDRAGDROP : 0);
			return this->target();
		}
	};

	enum class view : WORD {
		DETAILS   = LV_VIEW_DETAILS,
		ICON      = LV_VIEW_ICON,
		LIST      = LV_VIEW_LIST,
		SMALLICON = LV_VIEW_SMALLICON,
		TILE      = LV_VIEW_TILE
	};

private:
	subclass   _subclass;
	menu       _contextMenu;
	image_list _imgList16, _imgList32;
public:
	collection items;
	styler     style;

	~listview() { this->_contextMenu.destroy(); }

	listview() : _subclass(2), items(this), style(this) {
		this->_subclass.on_message(WM_GETDLGCODE, [&](params& p)->LRESULT {
			bool hasCtrl = (GetAsyncKeyState(VK_CONTROL) & 0x8000) != 0;
			BYTE vkey = static_cast<BYTE>(p.wParam);
			if (p.lParam && vkey == 'A' && hasCtrl) { // Ctrl+A to select all items
				p.wParam = 0; // prevent propagation, therefore beep
				ListView_SetItemState(this->hwnd(), -1, LVIS_SELECTED, LVIS_SELECTED);
				return DLGC_WANTCHARS;
			} else if (p.lParam && vkey == VK_RETURN) { // send Enter key to parent
				NMLVKEYDOWN nmlvkd = {
					{this->hwnd(), static_cast<WORD>(this->control_id()), LVN_KEYDOWN},
					VK_RETURN, 0
				};
				SendMessageW(GetAncestor(this->hwnd(), GA_PARENT),
					WM_NOTIFY, reinterpret_cast<WPARAM>(this->hwnd()),
					reinterpret_cast<LPARAM>(&nmlvkd) );
				p.wParam = 0; // prevent propagation, therefore beep
				return DLGC_WANTALLKEYS;
			} else if (p.lParam && vkey == VK_APPS) { // context menu keyboard key
				this->_show_context_menu(false);
			}
			return DefSubclassProc(this->hwnd(), p.message, p.wParam, p.lParam);
		});
		this->_subclass.on_message(WM_RBUTTONDOWN, [&](const params& p)->LRESULT {
			this->_show_context_menu(true);
			return 0;
		});
	}

	listview& assign(HWND hParent, int controlId) {
		this->native_control::assign(hParent, controlId);
		return this->_install_subclass();
	}

	listview& assign(const base::wnd* parent, int controlId) {
		return this->assign(parent->hwnd(), controlId);
	}

	listview& create(HWND hParent, int controlId, POINT pos, SIZE size, view viewType = view::DETAILS) {
		this->native_control::create(hParent, controlId, nullptr, pos, size, WC_LISTVIEW,
			WS_CHILD | WS_VISIBLE | WS_TABSTOP | static_cast<DWORD>(viewType),
			WS_EX_CLIENTEDGE); // for children, WS_BORDER gives old, flat drawing; always use WS_EX_CLIENTEDGE
		return this->_install_subclass();
	}

	listview& create(const base::wnd* parent, int controlId, POINT pos, SIZE size, view viewType = view::DETAILS) {
		return this->create(parent->hwnd(), controlId, pos, size, viewType);
	}

	listview& focus() {
		SetFocus(this->hwnd());
		return *this;
	}

	listview& enable(bool doEnable) {
		EnableWindow(this->hwnd(), doEnable);
		return *this;
	}

	listview& set_context_menu(int contextMenuId) {
		if (this->_contextMenu.hmenu()) {
			OutputDebugStringW(L"ERROR: listview context menu already assigned.\n");
		} else {
			this->_contextMenu.load_resource_submenu(contextMenuId, 0,
				GetParent(this->hwnd()));
		}
		return *this;
	}

	listview& set_redraw(bool doRedraw) {
		SendMessageW(this->hwnd(), WM_SETREDRAW,
			static_cast<WPARAM>(static_cast<BOOL>(doRedraw)), 0);
		return *this;
	}

	listview& set_view(view viewType) {
		ListView_SetView(this->hwnd(), static_cast<DWORD>(viewType));
		return *this;
	}

	view get_view() const {
		return static_cast<view>(ListView_GetView(this->hwnd()));
	}

	listview& icon_16_push(int iconId) {
		if (this->_create_image_list(true, false)) {
			this->_imgList16.load_from_resource(iconId, GetParent(this->hwnd()));
		}
		return *this;
	}

	listview& icon_16_push(const wchar_t* fileExtension) {
		if (this->_create_image_list(true, false)) {
			this->_imgList16.load_from_shell(fileExtension);
		}
		return *this;
	}

	listview& icon_32_push(int iconId) {
		if (this->_create_image_list(false, true)) {
			this->_imgList32.load_from_resource(iconId, GetParent(this->hwnd()));
		}
		return *this;
	}

	listview& icon_32_push(const wchar_t* fileExtension) {
		if (this->_create_image_list(false, true)) {
			this->_imgList32.load_from_shell(fileExtension);
		}
		return *this;
	}

	listview& icon_16_32_push(int iconId) {
		return this->icon_16_push(iconId)
			.icon_32_push(iconId);
	}

	listview& icon_16_32_push(const wchar_t* fileExtension) {
		return this->icon_16_push(fileExtension)
			.icon_32_push(fileExtension);
	}

	size_t column_count() const {
		return Header_GetItemCount(ListView_GetHeader(this->hwnd()));
	}

	listview& column_add(const wchar_t* caption, int cx) {
		LVCOLUMN lvc = { 0 };
		lvc.mask = LVCF_TEXT | LVCF_WIDTH;
		lvc.pszText = const_cast<wchar_t*>(caption);
		lvc.cx = cx;
		ListView_InsertColumn(this->hwnd(), 0xFFFF, &lvc);
		return *this;
	}

	listview& column_fit(size_t columnIndex) {
		size_t numCols = this->column_count();
		int cxUsed = 0;
		for (size_t i = 0; i < numCols; ++i) {
			if (i != columnIndex) {
				LVCOLUMN lvc = { 0 };
				lvc.mask = LVCF_WIDTH;
				ListView_GetColumn(this->hwnd(), i, &lvc); // retrieve cx of each column, except stretchee
				cxUsed += lvc.cx; // sum up
			}
		}

		RECT rc = { 0 };
		GetClientRect(this->hwnd(), &rc); // listview client area
		ListView_SetColumnWidth(this->hwnd(), columnIndex,
			rc.right /*- GetSystemMetrics(SM_CXVSCROLL)*/ - cxUsed); // fit the rest of available space
		return *this;
	}

	static std::vector<std::wstring> get_all_text(std::vector<item> items, size_t columnIndex = 0) {
		std::vector<std::wstring> texts;
		texts.reserve(items.size());
		for (item oneItem : items) {
			texts.emplace_back(oneItem.get_text(columnIndex));
		}
		return texts;
	}

private:
	bool _create_image_list(bool il16, bool il32) {
		// Imagelists are destroyed automatically:
		// http://www.catch22.net/tuts/sysimgq
		// http://www.autohotkey.com/docs/commands/ListView.htm
		if (il16 && !this->_imgList16.himagelist()) {
			if (!this->_imgList16.create({16, 16})) {
				OutputDebugStringW(L"ERROR: failed to create 16x16 listview image list.\n");
				return false;
			}
			ListView_SetImageList(this->hwnd(),
				this->_imgList16.himagelist(), LVSIL_SMALL); // associate imagelist to listview control
		}
		if (il32 && !this->_imgList32.himagelist()) {
			if (!this->_imgList32.create({32, 32})) {
				OutputDebugStringW(L"ERROR: failed to create 32x32 listview image list.\n");
				return false;
			}
			ListView_SetImageList(this->hwnd(),
				this->_imgList32.himagelist(), LVSIL_NORMAL);
		}
		return true;
	}

	listview& _install_subclass() {
		this->_subclass.install_subclass(this->hwnd());
		return *this;
	}

	int _show_context_menu(bool followCursor) {
		if (!this->_contextMenu.hmenu()) return -1; // no context menu assigned

		POINT coords = { 0 };
		int itemBelowCursor = -1;
		if (followCursor) { // usually fired with a right-click
			LVHITTESTINFO lvhti = { 0 };
			GetCursorPos(&lvhti.pt); // relative to screen
			ScreenToClient(this->hwnd(), &lvhti.pt); // now relative to listview
			ListView_HitTest(this->hwnd(), &lvhti); // item below cursor, if any
			coords = lvhti.pt;
			itemBelowCursor = lvhti.iItem; // -1 if none
			bool hasCtrl = (GetAsyncKeyState(VK_CONTROL) & 0x8000) != 0;
			bool hasShift = (GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0;
			if (itemBelowCursor != -1) { // an item was right-clicked
				if (!hasCtrl && !hasShift) {
					if ((ListView_GetItemState(this->hwnd(), itemBelowCursor, LVIS_SELECTED) & LVIS_SELECTED) == 0) {
						// If right-clicked item isn't currently selected, unselect all and select just it.
						ListView_SetItemState(this->hwnd(), -1, 0, LVIS_SELECTED);
						ListView_SetItemState(this->hwnd(), itemBelowCursor, LVIS_SELECTED, LVIS_SELECTED);
					}
					ListView_SetItemState(this->hwnd(), itemBelowCursor, LVIS_FOCUSED, LVIS_FOCUSED); // focus clicked
				}
			} else if (!hasCtrl && !hasShift) {
				ListView_SetItemState(this->hwnd(), -1, 0, LVIS_SELECTED); // unselect all
			}
			SetFocus(this->hwnd()); // because a right-click won't set the focus by default
		} else { // usually fired with the context menu keyboard key
			int itemFocused = ListView_GetNextItem(this->hwnd(), -1, LVNI_FOCUSED);
			if (itemFocused != -1 && ListView_IsItemVisible(this->hwnd(), itemFocused)) { // item focused and visible
				RECT rcItem = { 0 };
				ListView_GetItemRect(this->hwnd(), itemFocused, &rcItem, LVIR_BOUNDS); // relative to listview
				coords = { rcItem.left + 16, rcItem.top + (rcItem.bottom - rcItem.top) / 2 };
			} else { // no focused and visible item
				coords = { 6, 10 };
			}
		}

		// The popup menu is created with hDlg as parent, so the menu messages go to it.
		// The lvhti coordinates are relative to listview, and will be mapped into screen-relative.
		this->_contextMenu.show_at_point(GetParent(this->hwnd()), coords, this->hwnd());
		return itemBelowCursor; // -1 if none
	}
};

}//namespace wl