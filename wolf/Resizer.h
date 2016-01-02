/**
 * Part of WOLF - WinAPI Object Lambda Framework
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/wolf
 */

#pragma once
#include <functional>
#include <vector>
#include "WindowParent.h"

namespace wolf {

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
	WindowParent&         _parent;
	std::vector<Ctrl>     _ctrls;
	SIZE                  _szOrig;
	std::function<void()> _afterResize;
public:
	explicit Resizer(WindowParent *parent);
	Resizer& operator=(const Resizer& r) = delete;
	Resizer& operator=(Resizer&& r) = delete;
	Resizer& add(int ctrlId, Do modeHorz, Do modeVert);
	Resizer& add(HWND child, Do modeHorz, Do modeVert);
	Resizer& add(const Window *child, Do modeHorz, Do modeVert);
	Resizer& add(std::initializer_list<int> ctrlIds, Do modeHorz, Do modeVert);
	Resizer& add(std::initializer_list<HWND> children, Do modeHorz, Do modeVert);
	Resizer& add(std::initializer_list<const Window*> pChildren, Do modeHorz, Do modeVert);
	Resizer& afterResize(std::function<void()> callback);
private:
	void _addOne(HWND hCtrl, Do modeHorz, Do modeVert);
	void _setupOnce();
};

}//namespace wolf