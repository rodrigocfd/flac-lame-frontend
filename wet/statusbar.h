/**
 * Part of WET - WinAPI Easy Toolkit
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/wet
 */

#pragma once
#include "base_wnd.h"
#include "params.h"

/**
 * base_wnd <-- statusbar
 */

namespace wet {

class statusbar final : public base_wnd {
private:
	struct _part final {
		UINT sizePixels;
		UINT resizeWeight;
	};

	std::vector<_part> _parts;
	std::vector<int>   _rightEdges;

public:
	statusbar() = default;
	statusbar& operator=(const statusbar& sb) = delete;
	statusbar& operator=(statusbar&& sb) = delete;

	statusbar& create(HWND hParent) {
		if (this->base_wnd::hwnd()) {
			DBG(L"ERROR: statusbar already created.\n");
		} else {
			DWORD parentStyle = static_cast<DWORD>(GetWindowLongPtrW(hParent, GWL_STYLE));
			bool isStretch = (parentStyle & WS_MAXIMIZEBOX) != 0 ||
				(parentStyle & WS_SIZEBOX) != 0;

			this->base_wnd::operator=( CreateWindowExW(0, STATUSCLASSNAME, nullptr,
				(WS_CHILD | WS_VISIBLE) | (isStretch ? SBARS_SIZEGRIP : 0),
				0, 0, 0, 0, hParent, nullptr,
				reinterpret_cast<HINSTANCE>(GetWindowLongPtrW(hParent, GWLP_HINSTANCE)),
				nullptr) );
		}
		return *this;
	}

	statusbar& create(const base_wnd* parent) {
		return this->create(parent->hwnd());
	}

	void adjust(WPARAM wp, LPARAM lp) {
		// Intended to be called with parent's WM_SIZE processing.
		if (wp != SIZE_MINIMIZED && this->base_wnd::hwnd()) {
			int cx = LOWORD(lp); // available width
			SendMessageW(this->base_wnd::hwnd(), WM_SIZE, 0, 0); // tell statusbar to fit parent

			// Find the space to be divided among variable-width parts,
			// and total weight of variable-width parts.
			UINT totalWeight = 0;
			int  cxVariable = cx;
			for (const _part& onePart : this->_parts) {
				if (!onePart.resizeWeight) { // fixed-width?
					cxVariable -= onePart.sizePixels;
				} else {
					totalWeight += onePart.resizeWeight;
				}
			}

			// Fill right edges array with the right edge of each part.
			int cxTotal = cx;
			for (size_t i = this->_parts.size(); i-- > 0; ) {
				this->_rightEdges[i] = cxTotal;
				cxTotal -= (!this->_parts[i].resizeWeight) ? // fixed-width?
					this->_parts[i].sizePixels :
					static_cast<int>( (cxVariable / totalWeight) * this->_parts[i].resizeWeight );
			}
			SendMessageW(this->base_wnd::hwnd(), SB_SETPARTS, this->_rightEdges.size(),
				reinterpret_cast<LPARAM>(&this->_rightEdges[0]));
		}
	}

	void adjust(params p) {
		return this->adjust(p.wParam, p.lParam);
	}

	statusbar& add_fixed_part(UINT sizePixels) {
		if (this->base_wnd::hwnd()) {
			this->_parts.push_back({sizePixels, 0});
			this->_rightEdges.emplace_back(0);
			this->adjust(SIZE_RESTORED, MAKELPARAM(this->_get_parent_cx(), 0));
		}
		return *this;
	}

	statusbar& add_resizable_part(UINT resizeWeight) {
		// How resizeWeight works:
		// Suppose you have 3 parts, respectively with weights of 1, 1 and 2.
		// If available client area is 400px, respective part widths will be 100, 100 and 200px.
		// Zero weight means a fixed-width part, which internally should have sizePixels set.
		if (this->base_wnd::hwnd()) {
			this->_parts.push_back({0, resizeWeight});
			this->_rightEdges.emplace_back(0);
			this->adjust(SIZE_RESTORED, MAKELPARAM(this->_get_parent_cx(), 0));
		}
		return *this;
	}

	statusbar& set_text(const wchar_t* text, size_t iPart) {
		SendMessageW(this->base_wnd::hwnd(), SB_SETTEXT, MAKEWPARAM(MAKEWORD(iPart, 0), 0),
			reinterpret_cast<LPARAM>(text));
		return *this;
	}

	statusbar& set_text(const std::wstring& text, size_t iPart) {
		return this->set_text(text.c_str(), iPart);
	}

	std::wstring get_text(size_t iPart) const {
		int len = LOWORD(SendMessageW(this->base_wnd::hwnd(), SB_GETTEXTLENGTH, iPart, 0)) + 1;
		std::wstring buf(len, L'\0');
		SendMessageW(this->base_wnd::hwnd(), SB_GETTEXT, iPart, reinterpret_cast<LPARAM>(&buf[0]));
		buf.resize(len);
		return buf;
	}

	statusbar& statusbar::set_icon(HICON hIcon, size_t iPart) {
		SendMessageW(this->base_wnd::hwnd(), SB_SETICON, iPart, reinterpret_cast<LPARAM>(hIcon));
		return *this;
	}

private:
	int _get_parent_cx() {
		static int cx = 0; // cache, since parts are intended to be added during window creation only
		if (!cx && this->base_wnd::hwnd()) {
			RECT rc = { 0 };
			GetClientRect(this->base_wnd::parent().hwnd(), &rc);
			cx = rc.right;
		}
		return cx;
	}
};

}//namespace wet