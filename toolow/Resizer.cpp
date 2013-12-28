
#include "Resizer.h"

Resizer& Resizer::create(int numCtrls)
{
	_ctrls.realloc(numCtrls);
	return *this;
}

Resizer& Resizer::add(std::initializer_list<HWND> hChildren, Do modeHorz, Do modeVert)
{
	for(int i = 0; i < (int)hChildren.size(); ++i)
		this->_addOne(*(hChildren.begin() + i), modeHorz, modeVert);
	return *this;
}

Resizer& Resizer::add(std::initializer_list<int> ctrlIds, HWND hParent, Do modeHorz, Do modeVert)
{
	for(int i = 0; i < (int)ctrlIds.size(); ++i)
		this->_addOne(GetDlgItem(hParent, *(ctrlIds.begin() + i)), modeHorz, modeVert);
	return *this;
}

void Resizer::_addOne(HWND hCtrl, Do modeHorz, Do modeVert)
{
	if(_idxLastInserted >= _ctrls.size() - 1) // protection against buffer overflow
		_ctrls.realloc(_ctrls.size() + 1);

	if(_idxLastInserted == -1) { // first call to _addOne()
		RECT rcP;
		GetClientRect(GetParent(hCtrl), &rcP);
		_szOrig.cx = rcP.right;
		_szOrig.cy = rcP.bottom; // save original size of parent
	}

	_Ctrl *pCtrl = &_ctrls[++_idxLastInserted]; // current child control being added
	pCtrl->hWnd = hCtrl;
	pCtrl->modeHorz = modeHorz;
	pCtrl->modeVert = modeVert;

	GetWindowRect(pCtrl->hWnd, &pCtrl->rcOrig);
	ScreenToClient(GetParent(hCtrl), (POINT*)&pCtrl->rcOrig);
	ScreenToClient(GetParent(hCtrl), (POINT*)&pCtrl->rcOrig.right); // client coordinates relative to parent
}

void Resizer::doResize(WPARAM wp, LPARAM lp)
{
	int state = (int)wp;
	int cx = LOWORD(lp);
	int cy = HIWORD(lp);

	if(_ctrls.size() && state != SIZE_MINIMIZED) { // only if created() was called; if minimized, no need to resize
		HDWP hdwp = BeginDeferWindowPos(_ctrls.size());
		for(int i = 0; i < _ctrls.size(); ++i) {
			_Ctrl *pCtrl = &_ctrls[i]; // current child control being worked with

			UINT uFlags = SWP_NOZORDER;
			if(pCtrl->modeHorz == Do::REPOS && pCtrl->modeVert == Do::REPOS) // reposition both vert & horz
				uFlags |= SWP_NOSIZE;
			else if(pCtrl->modeHorz == Do::RESIZE && pCtrl->modeVert == Do::RESIZE) // resize both vert & horz
				uFlags |= SWP_NOMOVE;

			DeferWindowPos(hdwp, pCtrl->hWnd, 0,
				pCtrl->modeHorz == Do::REPOS ?
					cx - _szOrig.cx + pCtrl->rcOrig.left :
					pCtrl->rcOrig.left, // keep original pos
				pCtrl->modeVert == Do::REPOS ?
					cy - _szOrig.cy + pCtrl->rcOrig.top :
					pCtrl->rcOrig.top, // keep original pos
				pCtrl->modeHorz == Do::RESIZE ?
					cx - _szOrig.cx + pCtrl->rcOrig.right - pCtrl->rcOrig.left :
					pCtrl->rcOrig.right - pCtrl->rcOrig.left, // keep original width
				pCtrl->modeVert == Do::RESIZE ?
					cy - _szOrig.cy + pCtrl->rcOrig.bottom - pCtrl->rcOrig.top :
					pCtrl->rcOrig.bottom - pCtrl->rcOrig.top, // keep original height
				uFlags);
		}
		EndDeferWindowPos(hdwp);
	}
}