/**
 * Part of WOLF - WinAPI Object Lambda Framework
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/wolf
 */

#pragma once
#include "WindowParent.h"

namespace wolf {

class ProgressBar final : public Window {
public:
	ProgressBar();
	ProgressBar(HWND hwnd);
	ProgressBar(Window&& w);
	ProgressBar& operator=(HWND hwnd);
	ProgressBar& operator=(Window&& w);
	ProgressBar& create(const WindowParent *parent, int id, POINT pos, SIZE size);
	ProgressBar& create(HWND hParent, int id, POINT pos, SIZE size);
	ProgressBar& setRange(int minVal, int maxVal);
	ProgressBar& setRange(int minVal, size_t maxVal);
	ProgressBar& setPos(int pos);
	ProgressBar& setPos(size_t pos);
	ProgressBar& setPos(double pos);
	ProgressBar& setWaiting(bool isWaiting);
	int          getPos();
private:
	Window::getText;
	Window::setText;
};

}//namespace wolf