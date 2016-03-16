
#include "DlgRunnin.h"
#include "Convert.h"
#include "../winutil/Str.h"
#include "../winutil/Sys.h"
#include "../res/resource.h"
using std::wstring;
using std::vector;

DlgRunnin::DlgRunnin(
	TaskBarProgress&       taskBar,
	int                    numThreads,
	Target                 targetType,
	const vector<wstring>& files,
	bool                   delSrc,
	bool                   isVbr,
	const wstring&         quality,
	const FileIni&         ini,
	const wstring&         destFolder
)
	: _taskBar(taskBar), _numThreads(numThreads), _targetType(targetType), _files(files),
		_delSrc(delSrc), _isVbr(isVbr), _quality(quality), _ini(ini), _destFolder(destFolder),
		_curFile(0), _filesDone(0)
{
	setup.dialogId = DLG_RUNNIN;

	on_message(WM_INITDIALOG, [this](params p)->INT_PTR
	{
		_lbl  = { hwnd(), LBL_STATUS };
		_prog = { hwnd(), PRO_STATUS };
	
		_prog.setRange(0, _files.size());
		_taskBar.setPos(0);
		_lbl.setText( Str::format(L"0 of %d files finished...", _files.size()) ); // initial text
		_time0.setNow(); // start timer
		Sys::enableXButton(hwnd(), false);
	
		// Proceed to the file conversion straight away.
		int nFiles = static_cast<int>(_files.size());
		int batchSz = (_numThreads < nFiles) ? _numThreads : nFiles; // limit parallel processing
		for (int i = 0; i < batchSz; ++i) {
			Sys::thread([this]()->void {
				_doProcessNextFile();
			});
		}

		center_on_parent();
		return TRUE;
	});
}

void DlgRunnin::_doProcessNextFile()
{
	int index = _curFile++;
	if (index >= static_cast<int>(_files.size())) return;

	const wstring& file = _files[index];
	bool good = true;
	wstring err;

	switch (_targetType) {
	case Target::MP3:
		good = Convert::toMp3(_ini, file, _destFolder, _delSrc, _quality, _isVbr, &err);
		break;
	case Target::FLAC:
		good = Convert::toFlac(_ini, file, _destFolder, _delSrc, _quality, &err);
		break;
	case Target::WAV:
		good = Convert::toWav(_ini, file, _destFolder, _delSrc, &err);
	}

	if (!good) {
		_curFile = static_cast<int>(_files.size()); // error, so avoid further processing
		ui_thread([&]()->void {
			Sys::msgBox(hwnd(), L"Conversion failed",
				Str::format(L"File #%d:\n%s\n%s", index, file.c_str(), err.c_str()),
				MB_ICONERROR);
			_taskBar.clear();
			EndDialog(hwnd(), IDCANCEL);
		});
	} else {
		++_filesDone;

		ui_thread([this]()->void {
			_prog.setPos(_filesDone);
			_taskBar.setPos(_filesDone, _files.size());
			_lbl.setText( Str::format(L"%d of %d files finished...", _filesDone, _files.size()) );
		});
			
		if (_filesDone < static_cast<int>(_files.size())) { // more files to come
			_doProcessNextFile();
		} else { // finished all processing
			ui_thread([this]()->void {
				DateTime fin;
				Sys::msgBox(hwnd(), L"Conversion finished",
					Str::format(L"%d files processed in %.2f seconds.",
						_files.size(), static_cast<double>(fin.minus(_time0)) / 1000),
					MB_ICONINFORMATION);
				_taskBar.clear();
				EndDialog(hwnd(), IDOK);
			});
		}
	}
}