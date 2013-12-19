//
// GUI to FLAC and LAME command line tools.
// v2.0 rewritten from scratch in the whole Saturday, July 28, 2012.
// New FlacLameFE at Saturday, April 6, 2013.
// Into VS2013/C++11 at Thursday vacation, December 19, 2013.
//

#include <crtdbg.h>
#include "MainDialog.h"
#include "../res/resource.h"

int WINAPI wWinMain(HINSTANCE hInst, HINSTANCE h0, LPWSTR cmdLine, int cmdShow)
{
	int ret = 0;
	{
		MainDialog d;
		ret = d.run(hInst, cmdShow, DLG_MAIN, ICO_MAIN);
	}
	_ASSERT(!_CrtDumpMemoryLeaks());
	return ret;
}