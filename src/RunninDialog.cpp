
#include "RunninDialog.h"
#include "Convert.h"
using namespace wolf;
using namespace wolf::res;
using std::wstring;
using std::vector;

void RunninDialog::events() {

this->onMessage(WM_INITDIALOG, [&](WPARAM wp, LPARAM lp)->INT_PTR
{
	_lbl = this->getChild(LBL_STATUS);
	_prog = this->getChild(PRO_STATUS);
	
	_prog.setRange(0, _files.size());
	_lbl.setText( str::Sprintf(L"0 of %d files finished...", _files.size()) ); // initial text
	_time0.setNow(); // start timer
	this->setXButton(false);
	
	// Proceed to the file conversion straight away.
	int nFiles = static_cast<int>(_files.size());
	int batchSz = (_numThreads < nFiles) ? _numThreads : nFiles; // limit parallel processing
	for (int i = 0; i < batchSz; ++i) {
		sys::Thread([&]() {
			this->_doProcessNextFile();
		});
	}
	return TRUE;
});

}//events

void RunninDialog::_doProcessNextFile()
{
	int index = _curFile++;
	if (index >= static_cast<int>(_files.size())) return;

	const wstring& file = _files[index];
	bool good = true;
	wstring err;

	switch (_targetType) {
	case Target::MP3:
		good = Convert::ToMp3(_ini, file, _destFolder, _delSrc, _quality, _isVbr, &err);
		break;
	case Target::FLAC:
		good = Convert::ToFlac(_ini, file, _destFolder, _delSrc, _quality, &err);
		break;
	case Target::WAV:
		good = Convert::ToWav(_ini, file, _destFolder, _delSrc, &err);
	}

	if (!good) {
		_curFile = static_cast<int>(_files.size()); // error, so avoid further processing
		this->origThreadSync([&]() {
			this->messageBox(L"Conversion failed",
				str::Sprintf(L"File #%d:\n%s\n%s", index, file.c_str(), err.c_str()),
				MB_ICONERROR);
			this->endDialog(IDCANCEL);
		});
	} else {
		++_filesDone;

		this->origThreadSync([&]() {
			_prog.setPos(_filesDone);
			_lbl.setText( str::Sprintf(L"%d of %d files finished...", _filesDone, _files.size()) );
		});
			
		if (_filesDone < static_cast<int>(_files.size())) { // more files to come
			this->_doProcessNextFile();
		} else { // finished all processing
			this->origThreadSync([&]() {
				Date fin;
				this->messageBox(L"Conversion finished",
					str::Sprintf(L"%d files processed in %.2f seconds.",
						_files.size(), static_cast<double>(fin.minus(_time0)) / 1000),
					MB_ICONINFORMATION);
				this->endDialog(IDOK);
			});
		}
	}
}