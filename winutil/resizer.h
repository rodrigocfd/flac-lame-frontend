/**
 * Part of WinUtil - Windows Utilities
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winutil
 */

#pragma once
#include <vector>
#include <Windows.h>

namespace winutil {

class resizer final {
public:
	enum class go {
		REPOS,  // control size is fixed; control moves around anchored
		RESIZE, // control size stretches; control doesn't move
		NOTHING // control doesn't move or resize
	};
private:
	struct ctrl final {
		HWND hWnd;     // handle to child window
		RECT rcOrig;   // original coordinates relative to parent
		go   modeHorz; // horizontal mode
		go   modeVert; // vertical mode
	};

private:
	std::vector<ctrl> _ctrls;
	SIZE _szOrig;
public:
	resizer& add(HWND hChild, go modeHorz, go modeVert);
	resizer& add(std::initializer_list<HWND> hChildren, go modeHorz, go modeVert);
	resizer& add(HWND hParent, int ctrlId, go modeHorz, go modeVert);
	resizer& add(HWND hParent, std::initializer_list<int> ctrlIds, go modeHorz, go modeVert);
	void     arrange(WPARAM wp, LPARAM lp) const;
private:
	void _add_one(HWND hCtrl, go modeHorz, go modeVert);
};

}//namespace winutil