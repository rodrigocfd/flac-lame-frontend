/**
 * Part of WOLF - WinAPI Object Lambda Framework
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/wolf
 */

#include "Resizer.h"
using namespace wolf;
using std::function;
using std::initializer_list;

Resizer::Resizer(WindowParent *parent)
	: _parent(*parent), _szOrig({0,0})
{
}

Resizer& Resizer::add(int ctrlId, Do modeHorz, Do modeVert)
{
	this->_addOne(GetDlgItem(this->_parent.hWnd(), ctrlId), modeHorz, modeVert);
	return *this;
}

Resizer& Resizer::add(HWND child, Do modeHorz, Do modeVert)
{
	this->_addOne(child, modeHorz, modeVert);
	return *this;
}

Resizer& Resizer::add(const Window *child, Do modeHorz, Do modeVert)
{
	this->_addOne(child->hWnd(), modeHorz, modeVert);
	return *this;
}

Resizer& Resizer::add(initializer_list<int> ctrlIds, Do modeHorz, Do modeVert)
{
	this->_ctrls.reserve(this->_ctrls.size() + ctrlIds.size());
	for (int ctrlId : ctrlIds) {
		this->add(ctrlId, modeHorz, modeVert);
	}
	return *this;
}

Resizer& Resizer::add(initializer_list<HWND> children, Do modeHorz, Do modeVert)
{
	this->_ctrls.reserve(this->_ctrls.size() + children.size());
	for (HWND hChild : children) {
		this->add(hChild, modeHorz, modeVert);
	}
	return *this;
}

Resizer& Resizer::add(initializer_list<const Window*> children, Do modeHorz, Do modeVert)
{
	this->_ctrls.reserve(this->_ctrls.size() + children.size());
	for (const Window *pChild : children) {
		this->add(pChild, modeHorz, modeVert);
	}
	return *this;
}

Resizer& Resizer::afterResize(function<void()> callback)
{
	this->_afterResize = std::move(callback);
	return *this;
}

void Resizer::_addOne(HWND hCtrl, Do modeHorz, Do modeVert)
{
	this->_setupOnce();

	RECT rcC = { 0 };
	GetWindowRect(hCtrl, &rcC);
	ScreenToClient(this->_parent.hWnd(), reinterpret_cast<POINT*>(&rcC));
	ScreenToClient(this->_parent.hWnd(), reinterpret_cast<POINT*>(&rcC.right)); // client coordinates relative to parent
	this->_ctrls.push_back({ hCtrl, rcC, modeHorz, modeVert });
}

void Resizer::_setupOnce()
{
	if (!this->_ctrls.empty()) {
		return; // run once at the first _addOne() call
	}

	RECT rcP = { 0 };
	GetClientRect(this->_parent.hWnd(), &rcP);
	this->_szOrig.cx = rcP.right;
	this->_szOrig.cy = rcP.bottom; // save original size of parent

	this->_parent.onMessage(WM_SIZE, [this](WPARAM wp, LPARAM lp)->LRESULT { // equivalent of subclass parent
		int state = static_cast<int>(wp);
		int cx = LOWORD(lp);
		int cy = HIWORD(lp);
		if (this->_ctrls.size() && state != SIZE_MINIMIZED) { // only if created() was called; if minimized, no need to resize
			HDWP hdwp = BeginDeferWindowPos(static_cast<int>(this->_ctrls.size()));
			for (const Ctrl& ctrl : this->_ctrls) {
				UINT uFlags = SWP_NOZORDER;
				if (ctrl.modeHorz == Do::REPOS && ctrl.modeVert == Do::REPOS) { // reposition both vert & horz
					uFlags |= SWP_NOSIZE;
				} else if (ctrl.modeHorz == Do::RESIZE && ctrl.modeVert == Do::RESIZE) { // resize both vert & horz
					uFlags |= SWP_NOMOVE;
				}

				DeferWindowPos(hdwp, ctrl.hWnd, nullptr,
					ctrl.modeHorz == Do::REPOS ?
						cx - this->_szOrig.cx + ctrl.rcOrig.left :
						ctrl.rcOrig.left, // keep original pos
					ctrl.modeVert == Do::REPOS ?
						cy - this->_szOrig.cy + ctrl.rcOrig.top :
						ctrl.rcOrig.top, // keep original pos
					ctrl.modeHorz == Do::RESIZE ?
						cx - this->_szOrig.cx + ctrl.rcOrig.right - ctrl.rcOrig.left :
						ctrl.rcOrig.right - ctrl.rcOrig.left, // keep original width
					ctrl.modeVert == Do::RESIZE ?
						cy - this->_szOrig.cy + ctrl.rcOrig.bottom - ctrl.rcOrig.top :
						ctrl.rcOrig.bottom - ctrl.rcOrig.top, // keep original height
					uFlags);
			}
			EndDeferWindowPos(hdwp);
			if (this->_afterResize) {
				this->_afterResize(); // invoke user callback, if any
			}
		}
		return 0;
	});
}