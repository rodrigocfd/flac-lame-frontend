/**
 * Part of WOLF - WinAPI Object Lambda Framework
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/wolf
 */

#include "TaskBarProgress.h"
using namespace wolf;

TaskBarProgress::~TaskBarProgress()
{
	if (this->_bar) {
		this->_bar->Release();
		CoUninitialize();
	}
}

TaskBarProgress::TaskBarProgress(const Window *mainWindow)
	: TaskBarProgress(mainWindow->hWnd())
{
}

TaskBarProgress::TaskBarProgress(const Window& mainWindow)
	: TaskBarProgress(mainWindow.hWnd())
{
}

TaskBarProgress::TaskBarProgress(HWND hWndMainWindow)
	: _hWnd(hWndMainWindow), _bar(nullptr)
{
}

TaskBarProgress& TaskBarProgress::setPos(size_t percent, size_t total)
{
	this->_createOnce();
	this->_bar->SetProgressValue(this->_hWnd, static_cast<ULONGLONG>(percent),
		static_cast<ULONGLONG>(total));
	return *this;
}

TaskBarProgress& TaskBarProgress::setPos(double percent)
{
	return this->setPos(static_cast<size_t>(percent + 0.5), 100); // round
}

TaskBarProgress& TaskBarProgress::setWaiting(bool isWaiting)
{
	return this->_setState(isWaiting ? TBPF_INDETERMINATE : TBPF_NORMAL);
}

TaskBarProgress& TaskBarProgress::setPause(bool isPaused)
{
	return this->_setState(isPaused ? TBPF_PAUSED : TBPF_NORMAL);
}

TaskBarProgress& TaskBarProgress::setError(bool hasError)
{
	return this->_setState(hasError ? TBPF_ERROR : TBPF_NORMAL);
}

TaskBarProgress& TaskBarProgress::dismiss()
{
	return this->_setState(TBPF_NOPROGRESS);
}

void TaskBarProgress::_createOnce()
{
	if (!this->_bar) {
		CoInitialize(nullptr);
		CoCreateInstance(CLSID_TaskbarList, nullptr, CLSCTX_INPROC_SERVER,
			IID_ITaskbarList3, reinterpret_cast<LPVOID*>(&this->_bar));
	}
}

TaskBarProgress& TaskBarProgress::_setState(TBPFLAG state)
{
	this->_createOnce();
	this->_bar->SetProgressState(this->_hWnd, state);
	return *this;
}