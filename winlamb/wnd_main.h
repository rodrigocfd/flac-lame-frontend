/**
 * Part of WinLamb - Windows Lambda Library
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winlamb
 */

#pragma once
#include "wnd_proc.h"
#include <tchar.h>
#include <CommCtrl.h>
#pragma comment(lib, "Comctl32.lib")
#pragma comment(linker, \
	"\"/manifestdependency:type='Win32' " \
	"name='Microsoft.Windows.Common-Controls' " \
	"version='6.0.0.0' " \
	"processorArchitecture='*' " \
	"publicKeyToken='6595b64144ccf1df' " \
	"language='*'\"")

#define RUN(class_main) \
int APIENTRY _tWinMain(HINSTANCE hInst, HINSTANCE, LPTSTR, int cmdShow) \
{ \
	int ret = 0; \
	{ \
		class_main cm; \
		ret = cm.run(hInst, cmdShow); \
	} \
	_ASSERT(!_CrtDumpMemoryLeaks()); \
	return ret; \
}


/**
 * wnd <-- wnd_proc <-- wnd_main
 */

namespace winlamb {

template<typename traitsT>
class wnd_main : virtual public wnd_proc<traitsT> {
public:
	virtual ~wnd_main() = default;
	virtual int run(HINSTANCE hInst, int cmdShow) = 0;

protected:
	wnd_main()
	{
		this->wnd_proc::on_message(WM_NCDESTROY, [](wnd_proc::params p)->typename traitsT::ret_type {
			PostQuitMessage(0);
			return traitsT::processed_val;
		});
	}
};

}//namespace winlamb