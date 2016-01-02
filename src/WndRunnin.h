
#pragma once
#include "../wolf/wolf.h"

class WndRunnin final : public wolf::WindowModal {
public:
	enum class Target { NONE=0, MP3, FLAC, WAV };

private:
	wolf::TaskBarProgress&           _taskBar;
	wolf::Window                     _lbl;
	wolf::ProgressBar                _prog;
	int                              _numThreads;
	Target                           _targetType;
	const std::vector<std::wstring>& _files;
	bool                             _delSrc;
	bool                             _isVbr;
	const std::wstring&              _quality;
	const wolf::FileIni&             _ini;
	const std::wstring&              _destFolder;
	int                              _curFile, _filesDone;
	wolf::DateTime                   _time0;
public:
	WndRunnin(
		wolf::TaskBarProgress&           taskBar,
		int                              numThreads,
		Target                           targetType,
		const std::vector<std::wstring>& files,
		bool                             delSrc,
		bool                             isVbr,
		const std::wstring&              quality,
		const wolf::FileIni&             ini,
		const std::wstring&              destFolder
	);
private:
	void _doProcessNextFile();
};