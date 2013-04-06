//
// Fit size and position of a bunch of controls inside a window.
// The whole Sunday, June 5, 2011.
// From template to array on Saturday, March 3, 2012.
// Part of TOOLOW - Thin Object Oriented Layer Over Win32.
//

#pragma once
#include "Array.h"
#include <Windows.h>

class Resizer {
public:
	enum Behavior {
		REPOS,  // control size is fixed; control moves around anchored
		RESIZE, // control size stretches; control doesn't move
		RENONE  // control doesn't move or resize
	};

	Resizer() : _idxLastInserted(-1) { }

	Resizer& create(int numCtrls);
	Resizer& add(HWND hCtrl, Behavior modeHorz, Behavior modeVert);
	Resizer& add(HWND hParent, int ctrlId, Behavior modeHorz, Behavior modeVert);
	Resizer& addByHwnd(Behavior modeHorz, Behavior modeVert, int howMany, ...);
	Resizer& addById(Behavior modeHorz, Behavior modeVert, HWND hParent, int howMany, ...);
	void     doResize(WPARAM wp, LPARAM lp);

private:
	struct _Ctrl {
		HWND     hWnd;     // handle to child window
		RECT     rcOrig;   // original coordinates relative to parent
		Behavior modeHorz; // horizontal mode
		Behavior modeVert; // vertical mode
	};

	Array<_Ctrl> _ctrls;
	int          _idxLastInserted;
	SIZE         _szOrig;
};