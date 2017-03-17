/**
 * Part of WinLamb - Win32 API Lambda Library
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winlamb
 */

#pragma once
#include "base_native_control.h"
#include "base_styles.h"
#include "subclass.h"
#include "image_list.h"

/**
 * base_wnd <-- base_native_control <-- treeview
 */

namespace wl {

// Wrapper to treeview control from Common Controls library.
class treeview final : public base::native_control {
public:
	struct notif final {
		NFYDEC(asyncdraw, NMTVASYNCDRAW)
		NFYDEC(begindrag, NMTREEVIEW)
		NFYDEC(beginlabeledit, NMTVDISPINFO)
		NFYDEC(beginrdrag, NMTREEVIEW)
		NFYDEC(deleteitem, NMTREEVIEW)
		NFYDEC(endlabeledit, NMTVDISPINFO)
		NFYDEC(getdispinfo, NMTVDISPINFO)
		NFYDEC(getinfotip, NMTVGETINFOTIP)
		NFYDEC(itemchange, NMTVITEMCHANGE)
		NFYDEC(itemchanging, NMTVITEMCHANGE)
		NFYDEC(itemexpanded, NMTREEVIEW)
		NFYDEC(itemexpanding, NMTREEVIEW)
		NFYDEC(keydown, NMTVKEYDOWN)
		NFYDEC(selchanged, NMTREEVIEW)
		NFYDEC(selchanging, NMTREEVIEW)
		NFYDEC(setdispinfo, NMTVDISPINFO)
		NFYDEC(singleexpand, NMTREEVIEW)
		NFYDEC(click, NMHDR)
		NFYDEC(customdraw, NMTVCUSTOMDRAW)
		NFYDEC(dblclk, NMHDR)
		NFYDEC(killfocus, NMHDR)
		NFYDEC(rclick, NMHDR)
		NFYDEC(rdblclk, NMHDR)
		NFYDEC(return_, NMHDR)
		NFYDEC(setcursor, NMMOUSE)
		NFYDEC(setfocus, NMHDR)
	protected:
		notif() = default;
	};

	class item final {
	private:
		HTREEITEM _hTreeItem;
		treeview* _tree;
	public:
		item(HTREEITEM hTreeItem, treeview* pTree) : _hTreeItem(hTreeItem), _tree(pTree) { }
		item() : item(nullptr, nullptr) { }

		HTREEITEM htreeitem() const { return this->_hTreeItem; }

		item get_parent() const {
			return item(TreeView_GetParent(this->_tree->hwnd(), this->_hTreeItem), this->_tree);
		}

		bool is_root() const {
			return this->get_parent()._hTreeItem == nullptr;
		}	

		item get_first_child() const {
			return item(TreeView_GetChild(this->_tree->hwnd(), this->_hTreeItem), this->_tree);
		}

		std::vector<item> get_children() const {
			std::vector<item> children;
			item curIt = this->get_first_child();
			while (curIt.htreeitem()) {
				children.emplace_back(curIt);
				curIt = curIt.get_next_sibling();
			}
			return children;
		}

		item get_next_sibling() const {
			return item(TreeView_GetNextSibling(this->_tree->hwnd(), this->_hTreeItem), this->_tree);
		}

		item add_child(const wchar_t* text) {
			TVINSERTSTRUCT tvi = { 0 };
			tvi.hParent = this->_hTreeItem;
			tvi.hInsertAfter = TVI_LAST;
			tvi.itemex.mask = TVIF_TEXT;
			tvi.itemex.pszText = const_cast<wchar_t*>(text);

			return item(TreeView_InsertItem(this->_tree->hwnd(), &tvi), this->_tree); // return newly added item
		}

		item add_child(const std::wstring& text) {
			return this->add_child(text.c_str());
		}

		item& set_select() {
			TreeView_SelectItem(this->_tree->hwnd(), this->_hTreeItem);
			return *this;
		}

		std::wstring get_text() const {
			wchar_t tmpBuf[MAX_PATH] = { L'\0' }; // arbitrary length
			TVITEMEX tvi = { 0 };
			tvi.hItem = this->_hTreeItem;
			tvi.mask = TVIF_TEXT;
			tvi.cchTextMax = ARRAYSIZE(tmpBuf);
			tvi.pszText = tmpBuf;

			TreeView_GetItem(this->_tree->hwnd(), &tvi);
			return tvi.pszText;
		}

		item& set_text(const wchar_t* text) {
			TVITEMEX tvi = { 0 };
			tvi.hItem = this->_hTreeItem;
			tvi.mask = TVIF_TEXT;
			tvi.pszText = const_cast<wchar_t*>(text);

			TreeView_SetItem(this->_tree->hwnd(), &tvi);
			return *this;
		}

		item& set_text(const std::wstring& text) {
			return this->set_text(text.c_str());
		}

		LPARAM get_param() const {
			TVITEMEX tvi = { 0 };
			tvi.hItem = this->_hTreeItem;
			tvi.mask = TVIF_PARAM;

			TreeView_GetItem(this->_tree->hwnd(), &tvi);
			return tvi.lParam;
		}

		item& set_param(LPARAM lp) {
			TVITEMEX tvi = { 0 };
			tvi.hItem = this->_hTreeItem;
			tvi.mask = TVIF_PARAM;
			tvi.lParam = lp;

			TreeView_SetItem(this->_tree->hwnd(), &tvi);
			return *this;
		}
	};

	class styler final : public base::styles<treeview> {
	public:
		explicit styler(treeview* pTree) : styles(pTree) { }

		treeview& always_show_sel(bool doSet) {
			return this->styles::set_style(doSet, TVS_SHOWSELALWAYS);
		}

		treeview& lines_and_buttons(bool doSet) {
			return this->styles::set_style(doSet,
				TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS);
		}

		treeview& single_expand(bool doSet) {
			return this->styles::set_style(doSet, TVS_SINGLEEXPAND);
		}

		treeview& double_buffer(bool doSet) {
			TreeView_SetExtendedStyle(this->target().hwnd(),
				doSet ? TVS_EX_DOUBLEBUFFER : 0, TVS_EX_DOUBLEBUFFER);
			return this->target();
		}
	};

private:
	class _root final {
	private:
		treeview* _tree;
	public:
		_root(treeview* pTree) : _tree(pTree) { }

		item get_first_root() const {
			return item(TreeView_GetRoot(this->_tree->hwnd()), this->_tree);
		}

		std::vector<item> get_roots() const {
			std::vector<item> roots;
			item curIt = this->get_first_root();
			while (curIt.htreeitem()) {
				roots.emplace_back(curIt);
				curIt = curIt.get_next_sibling();
			}
			return roots;
		}

		item add_root(const wchar_t* caption) {
			TVINSERTSTRUCT tvi = { 0 };
			tvi.hParent = TVI_ROOT;
			tvi.hInsertAfter = TVI_LAST;
			tvi.itemex.mask = TVIF_TEXT;
			tvi.itemex.pszText = const_cast<wchar_t*>(caption);

			return item(TreeView_InsertItem(this->_tree->hwnd(), &tvi), this->_tree); // return newly added item
		}

		item add_root(const std::wstring& caption) {
			return this->add_root(caption.c_str());
		}
	};

public:
	_root  items;
	styler style;

	treeview() : items(this), style(this) { }

	treeview& assign(const base::wnd* parent, int controlId) {
		this->native_control::assign(parent, controlId);
		return *this;
	}

	treeview& create(const base::wnd* parent, int controlId, POINT pos, SIZE size) {
		this->native_control::create(parent, controlId, nullptr, pos, size, WC_TREEVIEW,
			WS_CHILD | WS_VISIBLE | WS_TABSTOP,
			WS_EX_CLIENTEDGE); // for children, WS_BORDER gives old, flat drawing; always use WS_EX_CLIENTEDGE
		return *this;
	}

	treeview& focus() {
		SetFocus(this->hwnd());
		return *this;
	}

	treeview& enable(bool doEnable) {
		EnableWindow(this->hwnd(), doEnable);
		return *this;
	}
};

}//namespace wl