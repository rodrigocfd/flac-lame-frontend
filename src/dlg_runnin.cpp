
#include "dlg_runnin.h"
#include "convert.h"
#include "../winutil/str.h"
#include "../winutil/sys.h"
#include "../res/resource.h"
using namespace winutil;
using std::wstring;
using std::vector;

dlg_runnin::dlg_runnin(
	taskbar_progress&      taskBar,
	int                    numThreads,
	target                 targetType,
	const vector<wstring>& files,
	bool                   delSrc,
	bool                   isVbr,
	const wstring&         quality,
	const file_ini&        ini,
	const wstring&         destFolder
)
	: _taskBar(taskBar), _numThreads(numThreads), _targetType(targetType), _files(files),
		_delSrc(delSrc), _isVbr(isVbr), _quality(quality), _ini(ini), _destFolder(destFolder),
		_curFile(0), _filesDone(0)
{
	setup.dialogId = DLG_RUNNIN;

	on_message(WM_INITDIALOG, [this](params p)->INT_PTR
	{
		_lbl  = GetDlgItem(hwnd(), LBL_STATUS);
		_prog = GetDlgItem(hwnd(), PRO_STATUS);
	
		_prog.set_range(0, _files.size());
		_taskBar.set_pos(0);
		_lbl.set_text( str::format(L"0 of %d files finished...", _files.size()) ); // initial text
		_time0.set_now(); // start timer
		sys::enable_x_button(hwnd(), false);
	
		// Proceed to the file conversion straight away.
		int nFiles = static_cast<int>(_files.size());
		int batchSz = (_numThreads < nFiles) ? _numThreads : nFiles; // limit parallel processing
		for (int i = 0; i < batchSz; ++i) {
			sys::thread([this]()->void {
				_process_next_file();
			});
		}

		center_on_parent();
		return TRUE;
	});
}

void dlg_runnin::_process_next_file()
{
	int index = _curFile++;
	if (index >= static_cast<int>(_files.size())) return;

	const wstring& file = _files[index];
	bool good = true;
	wstring err;

	switch (_targetType) {
	case target::MP3:
		good = Convert::toMp3(_ini, file, _destFolder, _delSrc, _quality, _isVbr, &err);
		break;
	case target::FLAC:
		good = Convert::toFlac(_ini, file, _destFolder, _delSrc, _quality, &err);
		break;
	case target::WAV:
		good = Convert::toWav(_ini, file, _destFolder, _delSrc, &err);
	}

	if (!good) {
		_curFile = static_cast<int>(_files.size()); // error, so avoid further processing
		ui_thread([&]()->void {
			sys::msg_box(hwnd(), L"Conversion failed",
				str::format(L"File #%d:\n%s\n%s", index, file.c_str(), err.c_str()),
				MB_ICONERROR);
			_taskBar.clear();
			EndDialog(hwnd(), IDCANCEL);
		});
	} else {
		++_filesDone;

		ui_thread([this]()->void {
			_prog.set_pos(_filesDone);
			_taskBar.set_pos(_filesDone, _files.size());
			_lbl.set_text( str::format(L"%d of %d files finished...", _filesDone, _files.size()) );
		});
			
		if (_filesDone < static_cast<int>(_files.size())) { // more files to come
			_process_next_file();
		} else { // finished all processing
			ui_thread([this]()->void {
				datetime fin;
				sys::msg_box(hwnd(), L"Conversion finished",
					str::format(L"%d files processed in %.2f seconds.",
						_files.size(), static_cast<double>(fin.minus(_time0)) / 1000),
					MB_ICONINFORMATION);
				_taskBar.clear();
				EndDialog(hwnd(), IDOK);
			});
		}
	}
}