
#pragma once
#include "../wet/dialog_modal.h"
#include "../wet/datetime.h"
#include "../wet/file_ini.h"
#include "../wet/label.h"
#include "../wet/progressbar.h"
#include "../wet/progress_taskbar.h"

class Dlg_Runnin final : public wet::dialog_modal {
public:
	enum class target { NONE = 0, MP3, FLAC, WAV };

private:
	wet::progress_taskbar&           _taskBar;
	wet::label                       _lbl;
	wet::progressbar                 _prog;
	int                              _numThreads;
	target                           _targetType;
	const std::vector<std::wstring>& _files;
	bool                             _delSrc;
	bool                             _isVbr;
	const std::wstring&              _quality;
	const wet::file_ini&             _ini;
	const std::wstring&              _destFolder;
	int                              _curFile, _filesDone;
	wet::datetime                    _time0;

public:
	Dlg_Runnin(
		wet::progress_taskbar&           taskBar,
		int                              numThreads,
		target                           targetType,
		const std::vector<std::wstring>& files,
		bool                             delSrc,
		bool                             isVbr,
		const std::wstring&              quality,
		const wet::file_ini&             ini,
		const std::wstring&              destFolder
	);

private:
	INT_PTR proc(wet::params p) override;
	void   _process_next_file();
};