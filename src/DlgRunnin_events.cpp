#define NOMINMAX // https://stackoverflow.com/a/5004874/6923555
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
	UINT batchSz = std::min(_opts.numThreads, numFiles);

	_taskbar.coCreateInstance(CLSID_TaskbarList);
	_taskbar->SetProgressState(GetParent(hWnd()), TBPF_NORMAL);
	_taskbar->SetProgressValue(GetParent(hWnd()), 0, numFiles);

	lib::NativeControl{this, LBL_STATUS}.setText(
		lib::str::fmt(L"Processing first %d file(s)...", batchSz));
	lib::ProgressBar{this, PRO_STATUS}.setRange(0, numFiles);

	for (UINT i = 0; i < batchSz; ++i) {
		this->runDetachedThread([this]() {
			_processNextFileDetached();
		});
	}
	return TRUE;
}
