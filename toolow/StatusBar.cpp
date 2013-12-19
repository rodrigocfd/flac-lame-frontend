
#include "StatusBar.h"

StatusBar& StatusBar::create(HWND hOwner, int numPartsItWillHave)
{
	// The owner is considered resizable if it has the maximize button.
	bool isStretch = (GetWindowLongPtr(hOwner, GWL_STYLE) & WS_MAXIMIZEBOX) != 0;
	
	_sb = CreateWindowEx(0, STATUSCLASSNAME, 0,
		(isStretch ? SBARS_SIZEGRIP : 0) | WS_CHILD | WS_VISIBLE,
		0, 0, 0, 0, hOwner, 0, (HINSTANCE)GetWindowLongPtr(hOwner, GWLP_HINSTANCE), 0);

	_parts.realloc(numPartsItWillHave);
	_lastInsertedPart = -1;
	_rightEdges.realloc(numPartsItWillHave); // only used during resizing; allocated through object life for performance
	return *this;
}

StatusBar& StatusBar::addFixedPart(BYTE sizePixels)
{
	++_lastInsertedPart;
	if(_lastInsertedPart >= _parts.size()) // buffer overrun protection
		return *this;

	_parts[_lastInsertedPart].sizePixels = sizePixels;
	_parts[_lastInsertedPart].resizeWeight = 0;

	if(_lastInsertedPart == _parts.size() - 1) {
		RECT rc = { 0 };
		_sb.getParent().getClientRect(&rc);
		this->_putParts(rc.right);
	}
	return *this;
}

StatusBar& StatusBar::addResizablePart(float resizeWeight)
{
	// How resizeWeight works:
	// Suppose you have 3 parts, respectively with weights of 1, 1 and 2.
	// If available client area is 400px, respective part widths will be 100, 100 and 200px.
	// Zero weight means a fixed-width part, which therefore should have sizePixels set (otherwise zero).

	++_lastInsertedPart;
	if(_lastInsertedPart >= _parts.size()) // buffer overrun protection
		return *this;

	_parts[_lastInsertedPart].sizePixels = 0;
	_parts[_lastInsertedPart].resizeWeight = resizeWeight;

	if(_lastInsertedPart == _parts.size() - 1) {
		RECT rc = { 0 };
		_sb.getParent().getClientRect(&rc);
		this->_putParts(rc.right);
	}
	return *this;
}

String StatusBar::getText(int iPart) const
{
	int len = LOWORD(_sb.sendMessage(SB_GETTEXTLENGTH, iPart, 0));
	String ret;
	ret.reserve(len);
	_sb.sendMessage(SB_GETTEXT, iPart, (LPARAM)ret.ptrAt(0));
	return ret;
}

void StatusBar::_putParts(int cx)
{
	_sb.sendMessage(WM_SIZE, 0, 0); // tell statusbar to fit parent

	// Find the space to be divided among variable-width parts,
	// and total weight of variable-width parts.
	float totalWeight = 0;
	int   cxVariable = cx, cxTotal = cx;

	for(int i = 0; i < _parts.size(); ++i) {
		if(!_parts[i].resizeWeight) // fixed-width?
			cxVariable -= _parts[i].sizePixels;
		else
			totalWeight += _parts[i].resizeWeight;
	}

	// Fill right edges array with the right edge of each part.
	for(int i = _parts.size() - 1; i >= 0; --i) {
		_rightEdges[i] = cxTotal;
		cxTotal -= (!_parts[i].resizeWeight) ? // fixed-width?
			_parts[i].sizePixels :
			(int)( (cxVariable / totalWeight) * _parts[i].resizeWeight );
	}

	_sb.sendMessage(SB_SETPARTS, _rightEdges.size(), (LPARAM)&_rightEdges[0]);
}