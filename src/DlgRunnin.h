
#pragma once
#include "../winlamb/dialog_modal.h"
#include "../winlamb/msg_thread.h"
#include "../winutil/datetime.h"
#include "../winutil/file_ini.h"
#include "../winutil/label.h"
#include "../winutil/progressbar.h"
#include "../winutil/taskbar_progress.h"

class DlgRunnin final : public winlamb::dialog_modal,
	public winlamb::dialog_msg_thread
{
public:
	enum class target { NONE = 0, MP3, FLAC, WAV };

private:
	winutil::taskbar_progress&       _taskBar;
	winutil::label                   _lbl;
	winutil::progressbar             _prog;
	int                              _numThreads;
	target                           _targetType;
	const std::vector<std::wstring>& _files;
	bool                             _delSrc;
	bool                             _isVbr;
	const std::wstring&              _quality;
	const winutil::file_ini&         _ini;
	const std::wstring&              _destFolder;
	int                              _curFile, _filesDone;
	winutil::datetime                _time0;
public:
	DlgRunnin(
		winutil::taskbar_progress&       taskBar,
		int                              numThreads,
		target                           targetType,
		const std::vector<std::wstring>& files,
		bool                             delSrc,
		bool                             isVbr,
		const std::wstring&              quality,
		const winutil::file_ini&         ini,
		const std::wstring&              destFolder
	);
private:
	void _process_next_file();
};