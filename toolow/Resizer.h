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
	enum class Do {
		REPOS,  // control size is fixed; control moves around anchored
		RESIZE, // control size stretches; control doesn't move
		NOTHING // control doesn't move or resize
	};

	Resizer() : _idxLastInserted(-1) { }

	Resizer& create(int numCtrls);
	Resizer& add(std::initializer_list<HWND> hChildren, Do modeHorz, Do modeVert);
	Resizer& add(std::initializer_list<int> ctrlIds, HWND hParent, Do modeHorz, Do modeVert);
	void     doResize(WPARAM wp, LPARAM lp);

private:
	void _addOne(HWND hCtrl, Do modeHorz, Do modeVert);

	struct _Ctrl {
		HWND hWnd;     // handle to child window
		RECT rcOrig;   // original coordinates relative to parent
		Do   modeHorz; // horizontal mode
		Do   modeVert; // vertical mode
	};

	Array<_Ctrl> _ctrls;
	int          _idxLastInserted;
	SIZE         _szOrig;
};