//
// Automation to WM_DROPFILES message handling.
// Part of TOOLOW - Thin Object Oriented Layer Over Win32.
//

#pragma once
#include "String.h"

class FileDrop {
public:
	FileDrop(HDROP hDrop) : _hDrop(hDrop), _count(::DragQueryFile(_hDrop, 0xFFFFFFFF, 0, 0)) { }
	~FileDrop()           { ::DragFinish(_hDrop); }
	int count() const     { return _count; }

	wchar_t* get(int i, wchar_t *pBuf) {
		::DragQueryFile(_hDrop, i, pBuf, MAX_PATH);
		return pBuf;
	}
	String* get(int i, String *pBuf) {
		pBuf->reserve(::DragQueryFile(_hDrop, i, 0, 0));
		::DragQueryFile(_hDrop, i, pBuf->ptrAt(0), pBuf->reserved() + 1);
		return pBuf;
	}

private:
	HDROP _hDrop;
	int   _count;
};