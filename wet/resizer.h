/**
 * Part of WET - WinAPI Easy Toolkit
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/wet
 */

#pragma once
#include "params.h"

namespace wet {

class resizer final {
public:
	enum class go {
		REPOS,  // control size is fixed; control moves around anchored
		RESIZE, // control size stretches; control doesn't move
		NOTHING // control doesn't move or resize
	};

private:
	struct _ctrl final {
		base_wnd wndChild;
		RECT     rcOrig;   // original coordinates relative to parent
		go       modeHorz; // horizontal mode
		go       modeVert; // vertical mode
	};

	std::vector<_ctrl> _ctrls;
	SIZE _szOrig;

public:
	resizer& add(base_wnd childCtrl, go modeHorz, go modeVert) {
		return this->_add_one(childCtrl, modeHorz, modeVert);
	}

	resizer& add(std::initializer_list<base_wnd> childCtrls, go modeHorz, go modeVert) {
		this->_ctrls.reserve(this->_ctrls.size() + childCtrls.size());
		for (const base_wnd& childC : childCtrls) {
			this->_add_one(childC, modeHorz, modeVert);
		}
		return *this;
	}

	resizer& add(const base_wnd* parent, int ctrlId, go modeHorz, go modeVert) {
		return this->_add_one(GetDlgItem(parent->hwnd(), ctrlId), modeHorz, modeVert);
	}

	resizer& add(const base_wnd* parent, std::initializer_list<int> ctrlIds, go modeHorz, go modeVert) {
		this->_ctrls.reserve(this->_ctrls.size() + ctrlIds.size());
		for (const int& ctrlId : ctrlIds) {
			this->_add_one(GetDlgItem(parent->hwnd(), ctrlId), modeHorz, modeVert);
		}
		return *this;
	}

	void arrange(WPARAM wp, LPARAM lp) const {
		// Intended to be called with parent's WM_SIZE processing.
		int state = static_cast<int>(wp);
		int cx    = LOWORD(lp);
		int cy    = HIWORD(lp);

		if (this->_ctrls.size() && state != SIZE_MINIMIZED) { // only if created() was called; if minimized, no need to resize
			HDWP hdwp = BeginDeferWindowPos(static_cast<int>(this->_ctrls.size()));
			for (const _ctrl& control : this->_ctrls) {
				UINT uFlags = SWP_NOZORDER;
				if (control.modeHorz == go::REPOS && control.modeVert == go::REPOS) { // reposition both vert & horz
					uFlags |= SWP_NOSIZE;
				} else if (control.modeHorz == go::RESIZE && control.modeVert == go::RESIZE) { // resize both vert & horz
					uFlags |= SWP_NOMOVE;
				}

				DeferWindowPos(hdwp, control.wndChild.hwnd(), nullptr,
					control.modeHorz == go::REPOS ?
					cx - this->_szOrig.cx + control.rcOrig.left :
					control.rcOrig.left, // keep original pos
					control.modeVert == go::REPOS ?
					cy - this->_szOrig.cy + control.rcOrig.top :
					control.rcOrig.top, // keep original pos
					control.modeHorz == go::RESIZE ?
					cx - this->_szOrig.cx + control.rcOrig.right - control.rcOrig.left :
					control.rcOrig.right - control.rcOrig.left, // keep original width
					control.modeVert == go::RESIZE ?
					cy - this->_szOrig.cy + control.rcOrig.bottom - control.rcOrig.top :
					control.rcOrig.bottom - control.rcOrig.top, // keep original height
					uFlags);
			}
			EndDeferWindowPos(hdwp);
		}
	}

	void arrange(params p) const {
		return this->arrange(p.wParam, p.lParam);
	}

private:
	resizer& _add_one(base_wnd childCtrl, go modeHorz, go modeVert) {
		HWND hParent = childCtrl.parent().hwnd();
		if (this->_ctrls.empty()) { // first call to _addOne()
			RECT rcP = { 0 };
			GetClientRect(hParent, &rcP);
			this->_szOrig.cx = rcP.right;
			this->_szOrig.cy = rcP.bottom; // save original size of parent
		}

		RECT rcCtrl = { 0 };
		GetWindowRect(childCtrl.hwnd(), &rcCtrl);
		this->_ctrls.push_back({childCtrl, rcCtrl, modeHorz, modeVert});
		ScreenToClient(hParent, reinterpret_cast<POINT*>(&this->_ctrls.back().rcOrig)); // client coordinates relative to parent
		ScreenToClient(hParent, reinterpret_cast<POINT*>(&this->_ctrls.back().rcOrig.right));
		return *this;
	}
};

}//namespace wet