/**
 * Part of WinLamb - Win32 API Lambda Library
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winlamb
 */

#pragma once
#include <vector>
#include "base_native_control.h"
#include "params.h"
#include <CommCtrl.h>

namespace wl {

class statusbar final {
public:
	struct notif final {
		NFYDEC(simplemodechange, NMHDR)
		NFYDEC(click, NMMOUSE)
		NFYDEC(dblclk, NMMOUSE)
		NFYDEC(rclick, NMMOUSE)
		NFYDEC(rdblclk, NMMOUSE)
	protected:
		notif() = default;
	};

private:
	struct _part final {
		UINT sizePixels;
		UINT resizeWeight;
	};

	base_native_control _control;
	std::vector<_part>  _parts;
	std::vector<int>    _rightEdges;

public:
	HWND hwnd() const { return this->_control.hwnd(); }

	statusbar& create(HWND hParent) {
		if (this->hwnd()) {
			OutputDebugStringW(L"ERROR: statusbar already created.\n");
		} else {
			DWORD parentStyle = static_cast<DWORD>(GetWindowLongPtrW(hParent, GWL_STYLE));
			bool isStretch = (parentStyle & WS_MAXIMIZEBOX) != 0 ||
				(parentStyle & WS_SIZEBOX) != 0;

			this->_control.create(hParent, 0, nullptr, {0,0}, {0,0}, STATUSCLASSNAME,
				(WS_CHILD | WS_VISIBLE) | (isStretch ? SBARS_SIZEGRIP : 0), 0);
		}
		return *this;
	}

	void adjust(const params& p) {
		// Intended to be called with parent's WM_SIZE processing.
		if (p.wParam != SIZE_MINIMIZED && this->hwnd()) {
			int cx = LOWORD(p.lParam); // available width
			SendMessageW(this->hwnd(), WM_SIZE, 0, 0); // tell statusbar to fit parent

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
			SendMessageW(this->hwnd(), SB_SETPARTS, this->_rightEdges.size(),
				reinterpret_cast<LPARAM>(&this->_rightEdges[0]));
		}
	}

	statusbar& add_fixed_part(UINT sizePixels) {
		if (this->hwnd()) {
			this->_parts.push_back({sizePixels, 0});
			this->_rightEdges.emplace_back(0);
			this->adjust(params{WM_SIZE, SIZE_RESTORED, MAKELPARAM(this->_get_parent_cx(), 0)});
		}
		return *this;
	}

	statusbar& add_resizable_part(UINT resizeWeight) {
		// How resizeWeight works:
		// Suppose you have 3 parts, respectively with weights of 1, 1 and 2.
		// If available client area is 400px, respective part widths will be 100, 100 and 200px.
		// Zero weight means a fixed-width part, which internally should have sizePixels set.
		if (this->hwnd()) {
			this->_parts.push_back({0, resizeWeight});
			this->_rightEdges.emplace_back(0);
			this->adjust(params{WM_SIZE, SIZE_RESTORED, MAKELPARAM(this->_get_parent_cx(), 0)});
		}
		return *this;
	}

	statusbar& set_text(const wchar_t* text, size_t iPart) {
		SendMessageW(this->hwnd(), SB_SETTEXT, MAKEWPARAM(MAKEWORD(iPart, 0), 0),
			reinterpret_cast<LPARAM>(text));
		return *this;
	}

	statusbar& set_text(const std::wstring& text, size_t iPart) {
		return this->set_text(text.c_str(), iPart);
	}

	std::wstring get_text(size_t iPart) const {
		int len = LOWORD(SendMessageW(this->hwnd(), SB_GETTEXTLENGTH, iPart, 0)) + 1;
		std::wstring buf(len, L'\0');
		SendMessageW(this->hwnd(), SB_GETTEXT, iPart, reinterpret_cast<LPARAM>(&buf[0]));
		buf.resize(len);
		return buf;
	}

	statusbar& set_icon(HICON hIcon, size_t iPart) {
		SendMessageW(this->hwnd(), SB_SETICON, iPart, reinterpret_cast<LPARAM>(hIcon));
		return *this;
	}

private:
	int _get_parent_cx() {
		static int cx = 0; // cache, since parts are intended to be added during window creation only
		if (!cx && this->hwnd()) {
			RECT rc = { 0 };
			GetClientRect(GetParent(this->hwnd()), &rc);
			cx = rc.right;
		}
		return cx;
	}
};

}//namespace wl