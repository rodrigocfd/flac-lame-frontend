//
// A window which is a container to two windows, splitted.
// Evening of Tuesday, November 8, 2011.
// To a self-contained window at Friday, December 2, 2011.
// Part of TOOLOW - Thin Object Oriented Layer Over Win32.
//

#pragma once
#include "Frame.h"

//__________________________________________________________________________________________________
// Base class.
//
class Splitter : public FrameCtrl {
public:
	virtual ~Splitter() = 0;
	virtual void create(HWND hLeft, HWND hRight, int id, Window *parent, int x, int y, int cx, int cy);
protected:
	virtual LRESULT msgHandler(UINT msg, WPARAM wp, LPARAM lp);

	HWND _hLeft, _hRight; // the two panel windows
	int  _pos;           // current coordinate of splitter grip, relative to container window
	bool _isDrag;       // flag; user is dragging grip now?

	struct _Metrics { enum {
		GRIPWIDTH = 10, // width of splitter grip
		PADDING = 0,   // padding surrounding each panel
		OFFSET = 90   // offset to guard from the borders of the window when moving splitter grip
	}; };
};

//__________________________________________________________________________________________________
// Vertical splitter.
//
class VSplitter : public Splitter {
public:
	void create(HWND hLeft, HWND hRight, int id, Window *parent, int x=0, int y=0, int cx=0, int cy=0);
private:
	LRESULT msgHandler(UINT msg, WPARAM wp, LPARAM lp);
};

//__________________________________________________________________________________________________
// Horizontal splitter.
//
class HSplitter : public Splitter {
public:
	void create(HWND hLeft, HWND hRight, int id, Window *parent, int x=0, int y=0, int cx=0, int cy=0);
private:
	LRESULT msgHandler(UINT msg, WPARAM wp, LPARAM lp);
};