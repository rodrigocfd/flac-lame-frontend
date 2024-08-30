#include "DlgRunnin.h"
#include "convert.h"
#include "../res/resource.h"
using std::scoped_lock, std::wstring;

void DlgRunnin::processNextFileDetached()
{
	UINT idxFile = 0;
	{
		scoped_lock lock{_mutex};
		idxFile = _idxNextFile++; // take the next available index and increment it
	}
	if (idxFile >= _opts.files.size()) return; // no more files to process

	if (!launchConvertProcess(idxFile)) return; // halt if an error occurred

	{	// Conversion finished, update UI and move to next file, if any.
		scoped_lock lock{_mutex};
		++_numFilesDone;
	}
	dlg.runUiThread([this]() {
		_taskbar->SetProgressValue(GetParent(hWnd()), _numFilesDone, _opts.files.size());
		lib::ProgressBar{this, PRO_STATUS}.setPos(_numFilesDone);
		lib::NativeControl{this, LBL_STATUS}.setText(
			lib::str::fmt(L"%u of %u files finished...", _numFilesDone, _opts.files.size()) );
	});

	if (_numFilesDone < _opts.files.size()) { // more files to come
		processNextFileDetached(); // reuse the same thread
	} else { // finished all processing
		dlg.runUiThread([this]() {
			lib::TimeCount::Duration dur = _time.now();
			wstring msg = lib::str::fmt(L"%d file(s) converted with %d thread(s) in %d:%02d.%03d.",
				_opts.files.size(), _opts.numThreads, dur.min, dur.sec, dur.ms);
			dlg.msgBox(L"Process finished", {}, msg, TDCBF_OK_BUTTON, TD_INFORMATION_ICON);

			_taskbar->SetProgressState(GetParent(hWnd()), TBPF_NOPROGRESS);
			EndDialog(hWnd(), 0);
		});
	}
}

bool DlgRunnin::launchConvertProcess(UINT idxFile)
{
	const wstring& file = _opts.files[idxFile];

	try {
		switch (_opts.target) {
		case Target::Mp3:
			convert::toMp3(_opts.ini, file, _opts.destFolder, _opts.delSrc, _opts.quality, _opts.isVbr);
			break;
		case Target::Flac:
			convert::toFlac(_opts.ini, file, _opts.destFolder, _opts.delSrc, _opts.quality);
			break;
		case Target::Wav:
			convert::toWav(_opts.ini, file, _opts.destFolder, _opts.delSrc);
		}
	} catch (const std::exception& e) {
		{
			scoped_lock lock{_mutex};
			_idxNextFile = static_cast<UINT>(_opts.files.size()); // prevent further processing
		}
		dlg.runUiThread([this, &idxFile, &file, &e]() {
			_taskbar->SetProgressState(GetParent(hWnd()), TBPF_ERROR);
			lib::ProgressBar prog{this, PRO_STATUS};
			prog.setState(PBST_ERROR);

			dlg.msgBox(L"Conversion failed", {},
				lib::str::fmt(L"File #%u:\n%s\n\n%s", idxFile, file, lib::str::toWide(e.what())),
				TDCBF_OK_BUTTON, TD_ERROR_ICON);

			_taskbar->SetProgressState(GetParent(hWnd()), TBPF_NOPROGRESS);
			prog.setState(PBST_NORMAL);
			EndDialog(hWnd(), 1);
		});
		return false;
	}

	return true;
}
