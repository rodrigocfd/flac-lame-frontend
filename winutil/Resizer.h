
#pragma once
#include <vector>
#include <Windows.h>

class Resizer final {
public:
	enum class Do {
		REPOS,  // control size is fixed; control moves around anchored
		RESIZE, // control size stretches; control doesn't move
		NOTHING // control doesn't move or resize
	};
private:
	struct Ctrl final {
		HWND hWnd;     // handle to child window
		RECT rcOrig;   // original coordinates relative to parent
		Do   modeHorz; // horizontal mode
		Do   modeVert; // vertical mode
	};

private:
	std::vector<Ctrl> _ctrls;
	SIZE _szOrig;

public:
	Resizer& add(HWND hChild, Do modeHorz, Do modeVert);
	Resizer& add(std::initializer_list<HWND> hChildren, Do modeHorz, Do modeVert);
	Resizer& add(HWND hParent, int ctrlId, Do modeHorz, Do modeVert);
	Resizer& add(HWND hParent, std::initializer_list<int> ctrlIds, Do modeHorz, Do modeVert);
	void     arrange(WPARAM wp, LPARAM lp) const;
private:
	void _addOne(HWND hCtrl, Do modeHorz, Do modeVert);
};