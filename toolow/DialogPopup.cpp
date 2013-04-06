
#include "Dialog.h"
#include "Font.h"

DialogPopup::~DialogPopup()
{
}

INT_PTR DialogPopup::msgHandler(UINT msg, WPARAM wp, LPARAM lp)
{
	static Font hSysFont; // to be shared among all regular dialog windows

	switch(msg)
	{
	case WM_INITDIALOG:
		if(!hSysFont.hFont()) {
			Font::Info nfof;
			Font::GetDefaultDialogFontInfo(&nfof);
			hSysFont.create(&nfof);
		}
		hSysFont.applyOnChildren(this->hWnd());
		break;
	}
	return Dialog::msgHandler(msg, wp, lp); // forward to parent class message handler
}

BOOL DialogPopup::endDialog(INT_PTR nResult)
{
	return EndDialog(this->hWnd(), nResult);
}