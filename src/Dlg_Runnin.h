
#pragma once
#include "../winlamb/dialog_modal.h"
#include "../winlamb-more/datetime.h"
#include "../winlamb-more/file_ini.h"
#include "../winlamb-more/label.h"
#include "../winlamb-more/progressbar.h"
#include "../winlamb-more/progress_taskbar.h"

class Dlg_Runnin final : public wl::dialog_modal {
public:
	enum class target { NONE = 0, MP3, FLAC, WAV };

private:
	wl::progress_taskbar&            m_taskbarProgr;
	wl::label                        m_lbl;
	wl::progressbar                  m_prog;
	size_t                           m_numThreads;
	target                           m_targetType;
	const std::vector<std::wstring>& m_files;
	bool                             m_delSrc;
	bool                             m_isVbr;
	std::wstring                     m_quality;
	const wl::file_ini&              m_ini;
	std::wstring                     m_destFolder;
	size_t                           m_curFile, m_filesDone;
	wl::datetime                     m_time0;

public:
	Dlg_Runnin(
		wl::progress_taskbar&            taskBar,
		size_t                           numThreads,
		target                           targetType,
		const std::vector<std::wstring>& files,
		bool                             delSrc,
		bool                             isVbr,
		std::wstring                     quality,
		const wl::file_ini&              ini,
		std::wstring                     destFolder
	);

private:
	void process_next_file();
	void enable_x_button(bool enable) const;
};