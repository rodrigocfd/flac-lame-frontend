#include "DlgRunnin.h"
#include "convert.h"
#include "../res/resource.h"

void DlgRunnin::_processNextFileDetached()
{
	UINT idxFile = 0;
	{
		std::scoped_lock lock{_mutex};
		idxFile = _idxNextFile++; // take the next available index and increment it
	}
	if (idxFile >= _opts.files.size()) return; // no more files to process

	if (!_launchConvertProcess(idxFile)) return; // halt if an error occurred

	{	// Conversion finished, update UI and move to next file, if any.
		std::scoped_lock lock{_mutex};
		++_numFilesDone;
	}
	this->runUiThread([this]() {
		_taskbar->SetProgressValue(GetParent(hWnd()), _numFilesDone, _opts.files.size());
		lib::ProgressBar{this, PRO_STATUS}.setPos(_numFilesDone);
		lib::NativeControl{this, LBL_STATUS}.setText(
			lib::str::fmt(L"%u of %u files finished...", _numFilesDone, _opts.files.size()) );
	});

	if (_numFilesDone < _opts.files.size()) { // more files to come
		_processNextFileDetached(); // reuse the same thread
	} else { // finished all processing
		this->runUiThread([this]() {
			_taskbar->SetProgressState(GetParent(hWnd()), TBPF_NOPROGRESS);
			EndDialog(hWnd(), 0);
		});
	}
}

bool DlgRunnin::_launchConvertProcess(UINT idxFile)
{
	const std::wstring& file = _opts.files[idxFile];
	std::wstring iniPath = convert::iniPath();

	try {
		switch (_opts.target) {
		case Target::Mp3:
			convert::toMp3(iniPath, file, _opts.destFolder, _opts.delSrc, _opts.quality, _opts.isVbr);
			break;
		case Target::Flac:
			convert::toFlac(iniPath, file, _opts.destFolder, _opts.delSrc, _opts.quality);
			break;
		case Target::Wav:
			convert::toWav(iniPath, file, _opts.destFolder, _opts.delSrc);
		}
	} catch (const std::exception& e) {
		{
			std::scoped_lock lock{_mutex};
			_idxNextFile = static_cast<UINT>(_opts.files.size()); // prevent further processing
		}
		this->runUiThread([this, &idxFile, &file, &e]() {
			_taskbar->SetProgressState(GetParent(hWnd()), TBPF_ERROR);
			lib::ProgressBar prog{this, PRO_STATUS};
			prog.setState(PBST_ERROR);

			this->sys.msgBox(L"Conversion failed", L"",
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
