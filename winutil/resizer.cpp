/**
 * Part of WinUtil - Windows Utilities
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winutil
 */

#include "resizer.h"
using namespace winutil;
using std::initializer_list;

resizer& resizer::add(HWND hChild, go modeHorz, go modeVert)
{
	_add_one(hChild, modeHorz, modeVert);
	return *this;
}

resizer& resizer::add(initializer_list<HWND> hChildren, go modeHorz, go modeVert)
{
	_ctrls.reserve(_ctrls.size() + hChildren.size());
	for (const HWND& hChild : hChildren) {
		_add_one(hChild, modeHorz, modeVert);
	}
	return *this;
}

resizer& resizer::add(HWND hParent, int ctrlId, go modeHorz, go modeVert)
{
	_add_one(GetDlgItem(hParent, ctrlId), modeHorz, modeVert);
	return *this;
}

resizer& resizer::add(HWND hParent, initializer_list<int> ctrlIds, go modeHorz, go modeVert)
{
	_ctrls.reserve(_ctrls.size() + ctrlIds.size());
	for (const int& ctrlId : ctrlIds) {
		_add_one(GetDlgItem(hParent, ctrlId), modeHorz, modeVert);
	}
	return *this;
}

void resizer::_add_one(HWND hCtrl, go modeHorz, go modeVert)
{
	HWND hParent = GetParent(hCtrl);

	if (_ctrls.empty()) { // first call to _addOne()
		RECT rcP = { 0 };
		GetClientRect(hParent, &rcP);
		_szOrig.cx = rcP.right;
		_szOrig.cy = rcP.bottom; // save original size of parent
	}

	RECT rcCtrl = { 0 };
	GetWindowRect(hCtrl, &rcCtrl);
	_ctrls.push_back({ hCtrl, rcCtrl, modeHorz, modeVert });
	ScreenToClient(hParent, reinterpret_cast<POINT*>(&_ctrls.back().rcOrig)); // client coordinates relative to parent
	ScreenToClient(hParent, reinterpret_cast<POINT*>(&_ctrls.back().rcOrig.right));
}

void resizer::arrange(WPARAM wp, LPARAM lp) const
{
	// Intended to be called with parent's WM_SIZE processing.

	int state = static_cast<int>(wp);
	int cx    = LOWORD(lp);
	int cy    = HIWORD(lp);

	if (_ctrls.size() && state != SIZE_MINIMIZED) { // only if created() was called; if minimized, no need to resize
		HDWP hdwp = BeginDeferWindowPos(static_cast<int>(_ctrls.size()));
		for (const ctrl& control : _ctrls) {
			UINT uFlags = SWP_NOZORDER;
			if (control.modeHorz == go::REPOS && control.modeVert == go::REPOS) { // reposition both vert & horz
				uFlags |= SWP_NOSIZE;
			} else if (control.modeHorz == go::RESIZE && control.modeVert == go::RESIZE) { // resize both vert & horz
				uFlags |= SWP_NOMOVE;
			}

			DeferWindowPos(hdwp, control.hWnd, nullptr,
				control.modeHorz == go::REPOS ?
				cx - _szOrig.cx + control.rcOrig.left :
				control.rcOrig.left, // keep original pos
				control.modeVert == go::REPOS ?
				cy - _szOrig.cy + control.rcOrig.top :
				control.rcOrig.top, // keep original pos
				control.modeHorz == go::RESIZE ?
				cx - _szOrig.cx + control.rcOrig.right - control.rcOrig.left :
				control.rcOrig.right - control.rcOrig.left, // keep original width
				control.modeVert == go::RESIZE ?
				cy - _szOrig.cy + control.rcOrig.bottom - control.rcOrig.top :
				control.rcOrig.bottom - control.rcOrig.top, // keep original height
				uFlags);
		}
		EndDeferWindowPos(hdwp);
	}
}