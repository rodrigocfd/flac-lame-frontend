
#include "Resizer.h"

Resizer& Resizer::create(int numCtrls)
{
	_ctrls.realloc(numCtrls);
	return *this;
}

Resizer& Resizer::add(HWND hCtrl, Resizer::Behavior modeHorz, Resizer::Behavior modeVert)
{
	if(_idxLastInserted >= _ctrls.size() - 1) // protection against buffer overflow
		_ctrls.realloc(_ctrls.size() + 1);

	if(_idxLastInserted == -1) { // first call to add()
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
	return *this;
}

Resizer& Resizer::add(HWND hParent, int ctrlId, Resizer::Behavior modeHorz, Resizer::Behavior modeVert)
{
	return this->add(GetDlgItem(hParent, ctrlId), modeHorz, modeVert);
}

Resizer& Resizer::addByHwnd(Resizer::Behavior modeHorz, Resizer::Behavior modeVert, int howMany, ...)
{
	va_list marker;
	va_start(marker, howMany);

	for(int i = 0; i < howMany; ++i)
		this->add(va_arg(marker, HWND), modeHorz, modeVert); // user should pass HWND of each child to be added
	
	va_end(marker);
	return *this;
}

Resizer& Resizer::addById(Resizer::Behavior modeHorz, Resizer::Behavior modeVert, HWND hParent, int howMany, ...)
{
	va_list marker;
	va_start(marker, howMany);

	for(int i = 0; i < howMany; ++i)
		this->add(GetDlgItem(hParent, va_arg(marker, int)), modeHorz, modeVert); // user should pass item ID of each child
	
	va_end(marker);
	return *this;
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
			if(pCtrl->modeHorz == REPOS && pCtrl->modeVert == REPOS) // reposition both vert & horz
				uFlags |= SWP_NOSIZE;
			else if(pCtrl->modeHorz == RESIZE && pCtrl->modeVert == RESIZE) // resize both vert & horz
				uFlags |= SWP_NOMOVE;

			DeferWindowPos(hdwp, pCtrl->hWnd, 0,
				pCtrl->modeHorz == REPOS ?
					cx - _szOrig.cx + pCtrl->rcOrig.left :
					pCtrl->rcOrig.left, // keep original pos
				pCtrl->modeVert == REPOS ?
					cy - _szOrig.cy + pCtrl->rcOrig.top :
					pCtrl->rcOrig.top, // keep original pos
				pCtrl->modeHorz == RESIZE ?
					cx - _szOrig.cx + pCtrl->rcOrig.right - pCtrl->rcOrig.left :
					pCtrl->rcOrig.right - pCtrl->rcOrig.left, // keep original width
				pCtrl->modeVert == RESIZE ?
					cy - _szOrig.cy + pCtrl->rcOrig.bottom - pCtrl->rcOrig.top :
					pCtrl->rcOrig.bottom - pCtrl->rcOrig.top, // keep original height
				uFlags);
		}
		EndDeferWindowPos(hdwp);
	}
}