/**
 * Part of WinLamb - Windows Lambda Library
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winlamb
 */

#pragma once
#include <Windows.h>
#include <CommCtrl.h>

namespace winlamb {

class handle {
protected:
	HWND _hWnd;
public:
	virtual ~handle() = default;

	handle()                : _hWnd(nullptr) { }
	handle(HWND h)          : _hWnd(h) { }
	handle(const handle& w) : _hWnd(w._hWnd) { }

	handle& operator=(HWND h)          { _hWnd = h; return *this; }
	handle& operator=(const handle& w) { _hWnd = w._hWnd; return *this; }

	HWND hwnd() const { return _hWnd; }
};

}//namespace winlamb



#pragma comment(lib, "Comctl32.lib")
#pragma comment(linker, \
	"\"/manifestdependency:type='Win32' " \
	"name='Microsoft.Windows.Common-Controls' " \
	"version='6.0.0.0' " \
	"processorArchitecture='*' " \
	"publicKeyToken='6595b64144ccf1df' " \
	"language='*'\"")

#define RUN(class_main) \
int WINAPI wWinMain(HINSTANCE hInst, HINSTANCE, wchar_t*, int cmdShow) { \
	int ret = 0; \
	{	class_main cm; \
		ret = cm.run(hInst, cmdShow); } \
	_ASSERT(!_CrtDumpMemoryLeaks()); \
	return ret; \
}