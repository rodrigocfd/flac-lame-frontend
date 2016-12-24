/**
 * Part of WET - WinAPI Easy Toolkit
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/wet
 */

#pragma once
#include "base_wnd.h"
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
int APIENTRY wWinMain(HINSTANCE hInst, HINSTANCE, LPWSTR, int cmdShow) \
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
 * base_wnd <-- base_wnd_main
 */

namespace wet {

class base_wnd_main : virtual public base_wnd {
public:
	virtual int run(HINSTANCE hInst, int cmdShow) = 0;

protected:
	base_wnd_main() = default;
};

}//namespace wet