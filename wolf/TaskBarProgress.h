/**
 * Part of WOLF - WinAPI Object Lambda Framework
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/wolf
 */

#pragma once
#include "Window.h"
#include <ShObjIdl.h>

namespace wolf {

class TaskBarProgress final {
private:
	HWND           _hWnd;
	ITaskbarList3 *_bar;
public:
	~TaskBarProgress();
	explicit TaskBarProgress(const Window *mainWindow);
	explicit TaskBarProgress(const Window& mainWindow);
	explicit TaskBarProgress(HWND hWndMainWindow);
	TaskBarProgress& setPos(size_t percent, size_t total);
	TaskBarProgress& setPos(double percent);
	TaskBarProgress& setWaiting(bool isWaiting);
	TaskBarProgress& setPause(bool isPaused);
	TaskBarProgress& setError(bool hasError);
	TaskBarProgress& dismiss();
private:
	void             _createOnce();
	TaskBarProgress& _setState(TBPFLAG state);
};

}//namespace wolf