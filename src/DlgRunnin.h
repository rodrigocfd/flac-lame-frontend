
#pragma once
#include "../winlamb/dialog_modal.h"
#include "../winutil/DateTime.h"
#include "../winutil/FileIni.h"
#include "../winutil/Label.h"
#include "../winutil/ProgressBar.h"
#include "../winutil/TaskBarProgress.h"

class DlgRunnin final : public winlamb::dialog_modal {
public:
	enum class Target { NONE = 0, MP3, FLAC, WAV };

private:
	TaskBarProgress&                 _taskBar;
	Label                            _lbl;
	ProgressBar                      _prog;
	int                              _numThreads;
	Target                           _targetType;
	const std::vector<std::wstring>& _files;
	bool                             _delSrc;
	bool                             _isVbr;
	const std::wstring&              _quality;
	const FileIni&                   _ini;
	const std::wstring&              _destFolder;
	int                              _curFile, _filesDone;
	DateTime                         _time0;
public:
	DlgRunnin(
		TaskBarProgress&                 taskBar,
		int                              numThreads,
		Target                           targetType,
		const std::vector<std::wstring>& files,
		bool                             delSrc,
		bool                             isVbr,
		const std::wstring&              quality,
		const FileIni&                   ini,
		const std::wstring&              destFolder
	);
private:
	void _doProcessNextFile();
};