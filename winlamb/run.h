/**
 * Part of WinLamb - Windows Lambda Library
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winlamb
 */

#pragma once
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
int WINAPI wWinMain(HINSTANCE hInst, HINSTANCE, PWSTR, int cmdShow) \
{ \
	int ret = 0; \
	{ \
		class_main cm; \
		ret = cm.run(hInst, cmdShow); \
	} \
	_ASSERT(!_CrtDumpMemoryLeaks()); \
	return ret; \
}