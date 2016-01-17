
#include "Resizer.h"
using std::initializer_list;

Resizer& Resizer::add(HWND hChild, Do modeHorz, Do modeVert)
{
	_addOne(hChild, modeHorz, modeVert);
	return *this;
}

Resizer& Resizer::add(initializer_list<HWND> hChildren, Do modeHorz, Do modeVert)
{
	_ctrls.reserve(_ctrls.size() + hChildren.size());
	for (const HWND& hChild : hChildren) {
		_addOne(hChild, modeHorz, modeVert);
	}
	return *this;
}

Resizer& Resizer::add(HWND hParent, int ctrlId, Do modeHorz, Do modeVert)
{
	_addOne(GetDlgItem(hParent, ctrlId), modeHorz, modeVert);
	return *this;
}

Resizer& Resizer::add(HWND hParent, initializer_list<int> ctrlIds, Do modeHorz, Do modeVert)
{
	_ctrls.reserve(_ctrls.size() + ctrlIds.size());
	for (const int& ctrlId : ctrlIds) {
		_addOne(GetDlgItem(hParent, ctrlId), modeHorz, modeVert);
	}
	return *this;
}

void Resizer::_addOne(HWND hCtrl, Do modeHorz, Do modeVert)
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

void Resizer::arrange(WPARAM wp, LPARAM lp) const
{
	// Intended to be called with parent's WM_SIZE processing.

	int state = static_cast<int>(wp);
	int cx    = LOWORD(lp);
	int cy    = HIWORD(lp);

	if (_ctrls.size() && state != SIZE_MINIMIZED) { // only if created() was called; if minimized, no need to resize
		HDWP hdwp = BeginDeferWindowPos(static_cast<int>(_ctrls.size()));
		for (const Ctrl& ctrl : _ctrls) {
			UINT uFlags = SWP_NOZORDER;
			if (ctrl.modeHorz == Do::REPOS && ctrl.modeVert == Do::REPOS) { // reposition both vert & horz
				uFlags |= SWP_NOSIZE;
			} else if (ctrl.modeHorz == Do::RESIZE && ctrl.modeVert == Do::RESIZE) { // resize both vert & horz
				uFlags |= SWP_NOMOVE;
			}

			DeferWindowPos(hdwp, ctrl.hWnd, nullptr,
				ctrl.modeHorz == Do::REPOS ?
				cx - _szOrig.cx + ctrl.rcOrig.left :
				ctrl.rcOrig.left, // keep original pos
				ctrl.modeVert == Do::REPOS ?
				cy - _szOrig.cy + ctrl.rcOrig.top :
				ctrl.rcOrig.top, // keep original pos
				ctrl.modeHorz == Do::RESIZE ?
				cx - _szOrig.cx + ctrl.rcOrig.right - ctrl.rcOrig.left :
				ctrl.rcOrig.right - ctrl.rcOrig.left, // keep original width
				ctrl.modeVert == Do::RESIZE ?
				cy - _szOrig.cy + ctrl.rcOrig.bottom - ctrl.rcOrig.top :
				ctrl.rcOrig.bottom - ctrl.rcOrig.top, // keep original height
				uFlags);
		}
		EndDeferWindowPos(hdwp);
	}
}