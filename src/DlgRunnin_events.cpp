#include <thread>
#include "DlgRunnin.h"
#include "convert.h"
#include "../res/resource.h"

INT_PTR DlgRunnin::dlgProc(UINT uMsg, WPARAM wp, LPARAM lp)
{
	switch (uMsg) {
		case WM_INITDIALOG: return onInitDialog();
		case WM_CLOSE:      return TRUE; // don't call EndDialog(), so user can't close it
		default:            return FALSE;
	}
}

INT_PTR DlgRunnin::onInitDialog()
{
	auto numFiles = static_cast<UINT>(_opts.files.size());

	_taskbar.coCreateInstance(CLSID_TaskbarList);
	_taskbar->SetProgressState(GetParent(hWnd()), TBPF_NORMAL);
	_taskbar->SetProgressValue(GetParent(hWnd()), 0, numFiles);

	lib::NativeControl{this, LBL_STATUS}.setText(
		lib::str::fmt(L"Processing first %d file(s)...", _opts.numThreads));
	lib::ProgressBar{this, PRO_STATUS}.setRange(0, numFiles);
	_time.restart();

	for (UINT i = 0; i < _opts.numThreads; ++i) {
		dlg.runDetachedThread([this]() {
			_processNextFileDetached();
		});
	}
	return TRUE;
}
