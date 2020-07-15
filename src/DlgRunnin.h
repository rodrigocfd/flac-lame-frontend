
#pragma once
#include "../winlamb/dialog_modal.h"
#include "../winlamb/file_ini.h"
#include "../winlamb/label.h"
#include "../winlamb/progressbar.h"
#include "../winlamb/progress_taskbar.h"

class DlgRunnin final : public wl::dialog_modal {
public:
	enum class target { NONE = 0, MP3, FLAC, WAV };

	struct runnin_options final {
		std::vector<std::wstring> files;
		size_t                    numThreads = 2;
		target                    targetType = target::NONE;
		bool                      delSrc = false;
		bool                      isVbr = false;
		std::wstring              quality;
		std::wstring              destFolder;
	};

private:
	wl::progress_taskbar& mTaskbarProgr;
	const wl::file_ini&   mIniFile;
	wl::label             mLbl;
	wl::progressbar       mProg;
	size_t                mCurFile = 0, mFilesDone = 0;
	wl::datetime          mTime0;

public:
	runnin_options opts;
	DlgRunnin(wl::progress_taskbar& taskbarProgr, const wl::file_ini& iniFile);

private:
	void processNextFile();
};