
#include "WndRunnin.h"
#include "Convert.h"
#include "../res/resource.h"
using namespace wolf;
using std::wstring;
using std::vector;

WndRunnin::WndRunnin(
	WindowMain                      *wmain,
	int                              numThreads,
	Target                           targetType,
	const std::vector<std::wstring>& files,
	bool                             delSrc,
	bool                             isVbr,
	const std::wstring&              quality,
	const wolf::FileIni&             ini,
	const std::wstring&              destFolder
)
	: _taskBar(wmain), _numThreads(numThreads), _targetType(targetType), _files(files),
		_delSrc(delSrc), _isVbr(isVbr), _quality(quality), _ini(ini), _destFolder(destFolder),
		_curFile(0), _filesDone(0)
{
	this->setup.dialogId = DLG_RUNNIN;

	this->onMessage(WM_CREATE, [this](WPARAM wp, LPARAM lp)->LRESULT
	{
		_lbl = this->getChild(LBL_STATUS);
		_prog = this->getChild(PRO_STATUS);
	
		_prog.setRange(0, _files.size());
		_taskBar.setPos(0U);
		_lbl.setText( Str::format(L"0 of %d files finished...", _files.size()) ); // initial text
		_time0.setNow(); // start timer
		EnableMenuItem(GetSystemMenu(this->hWnd(), FALSE),
			SC_CLOSE, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED); // disable X button
	
		// Proceed to the file conversion straight away.
		int nFiles = static_cast<int>(_files.size());
		int batchSz = (_numThreads < nFiles) ? _numThreads : nFiles; // limit parallel processing
		for (int i = 0; i < batchSz; ++i) {
			Sys::thread([this]()->void {
				this->_doProcessNextFile();
			});
		}
		return 0;
	});
}

void WndRunnin::_doProcessNextFile()
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
		this->guiThread([&]()->void {
			Sys::msgBox(this, L"Conversion failed",
				Str::format(L"File #%d:\n%s\n%s", index, file.c_str(), err.c_str()),
				MB_ICONERROR);
			_taskBar.dismiss();
			this->sendMessage(WM_CLOSE, 0, 0);
		});
	} else {
		++_filesDone;

		this->guiThread([&]()->void {
			_prog.setPos(_filesDone);
			_taskBar.setPos(_filesDone, _files.size());
			_lbl.setText( Str::format(L"%d of %d files finished...", _filesDone, _files.size()) );
		});
			
		if (_filesDone < static_cast<int>(_files.size())) { // more files to come
			this->_doProcessNextFile();
		} else { // finished all processing
			this->guiThread([&]()->void {
				DateTime fin;
				Sys::msgBox(this, L"Conversion finished",
					Str::format(L"%d files processed in %.2f seconds.",
						_files.size(), static_cast<double>(fin.minus(_time0)) / 1000),
					MB_ICONINFORMATION);
				_taskBar.dismiss();
				this->sendMessage(WM_CLOSE, 0, 0);
			});
		}
	}
}