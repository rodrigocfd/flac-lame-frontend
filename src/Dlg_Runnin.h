
#pragma once
#include <winlamb/dialog_modal.h>
#include <winlamb/file_ini.h>
#include <winlamb/label.h>
#include <winlamb/progressbar.h>
#include <winlamb/progress_taskbar.h>

class Dlg_Runnin final : public wl::dialog_modal {
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
	wl::progress_taskbar& m_taskbarProgr;
	const wl::file_ini&   m_iniFile;
	wl::label             m_lbl;
	wl::progressbar       m_prog;
	size_t                m_curFile = 0, m_filesDone = 0;
	wl::datetime          m_time0;

public:
	runnin_options opts;
	Dlg_Runnin(wl::progress_taskbar& taskbarProgr, const wl::file_ini& iniFile);

private:
	void process_next_file();
};