
#pragma once
#include "../wolf/wolf.h"
#include "../res/resource.h"

class RunninDialog final : public wolf::DialogModal {
public:
	enum class Target { NONE=0, MP3, FLAC, WAV };

public:
	RunninDialog(
		int                              numThreads,
		Target                           targetType,
		const std::vector<std::wstring>& files,
		bool                             delSrc,
		bool                             isVbr,
		const std::wstring&              quality,
		const wolf::file::Ini&           ini,
		const std::wstring&              destFolder )
			: DialogModal(DLG_RUNNIN),
			_numThreads(numThreads), _targetType(targetType), _files(files), _delSrc(delSrc),
			_isVbr(isVbr), _quality(quality), _ini(ini), _destFolder(destFolder), _curFile(0),
			_filesDone(0) { }
private:
	void events() override;
	void _doProcessNextFile();

	wolf::Window                     _lbl;
	wolf::ctrl::ProgressBar          _prog;
	int                              _numThreads;
	Target                           _targetType;
	const std::vector<std::wstring>& _files;
	bool                             _delSrc;
	bool                             _isVbr;
	const std::wstring&              _quality;
	const wolf::file::Ini&           _ini;
	const std::wstring&              _destFolder;
	int                              _curFile, _filesDone;
	wolf::res::Date                  _time0;
};