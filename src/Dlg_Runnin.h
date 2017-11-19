
#pragma once
#include "std.h"
#include <winlamb/dialog_modal.h>
#include <winlamb/file_ini.h>
#include <winlamb/label.h>
#include <winlamb/progressbar.h>
#include <winlamb/progress_taskbar.h>

class Dlg_Runnin final : public wl::dialog_modal {
public:
	enum class target { NONE = 0, MP3, FLAC, WAV };

private:
	wl::progress_taskbar&  m_taskbarProgr;
	wl::label              m_lbl;
	wl::progressbar        m_prog;
	size_t                 m_numThreads;
	target                 m_targetType;
	const vector<wstring>& m_files;
	bool                   m_delSrc;
	bool                   m_isVbr;
	wstring                m_quality;
	const wl::file_ini&    m_ini;
	wstring                m_destFolder;
	size_t                 m_curFile, m_filesDone;
	wl::datetime           m_time0;

public:
	Dlg_Runnin(
		wl::progress_taskbar&  taskBar,
		size_t                 numThreads,
		target                 targetType,
		const vector<wstring>& files,
		bool                   delSrc,
		bool                   isVbr,
		wstring                quality,
		const wl::file_ini&    ini,
		wstring                destFolder
	);

private:
	void process_next_file();
};