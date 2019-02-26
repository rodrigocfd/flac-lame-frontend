
#include "Dlg_Runnin.h"
#include <winlamb/str.h>
#include <winlamb/sysdlg.h>
#include "Convert.h"
#include "../res/resource.h"
using std::wstring;
using namespace wl;

Dlg_Runnin::Dlg_Runnin(progress_taskbar& taskbarProgr, const file_ini& iniFile)
	: m_taskbarProgr(taskbarProgr), m_iniFile(iniFile)
{
	setup.dialogId = DLG_RUNNIN;

	on_message(WM_INITDIALOG, [&](params)
	{
		m_lbl.assign(this, LBL_STATUS);
		m_prog.assign(this, PRO_STATUS);

		m_prog.set_range(0, opts.files.size());
		m_taskbarProgr.set_pos(0);
		m_lbl.set_text( str::format(L"0 of %u files finished...", opts.files.size()) ); // initial text
		m_time0.set_now(); // start timer

		// Proceed to the file conversion straight away.
		size_t batchSz = (opts.numThreads < opts.files.size()) ?
			opts.numThreads : opts.files.size(); // limit parallel processing

		for (size_t i = 0; i < batchSz; ++i) {
			run_thread_detached([&]() {
				process_next_file();
			});
		}

		center_on_parent();
		return TRUE;
	});

	on_message(WM_CLOSE, [](params)
	{
		return TRUE; // don't close the dialog, EndDialog() not called
	});
}

void Dlg_Runnin::process_next_file()
{
	size_t curIndex = m_curFile++;
	if (curIndex >= opts.files.size()) return;

	const wstring& file = opts.files[curIndex];

	try {
		switch (opts.targetType) {
		case target::MP3:
			Convert::to_mp3(m_iniFile, file, opts.destFolder, opts.delSrc, opts.quality, opts.isVbr);
			break;
		case target::FLAC:
			Convert::to_flac(m_iniFile, file, opts.destFolder, opts.delSrc, opts.quality);
			break;
		case target::WAV:
			Convert::to_wav(m_iniFile, file, opts.destFolder, opts.delSrc);
		}
	} catch (const std::exception& e) {
		m_curFile = opts.files.size(); // error, so avoid further processing
		run_thread_ui([&]() {
			sysdlg::msgbox(this, L"Conversion failed",
				str::format(L"File #%u:\n%s\n%s",
					curIndex, file, str::to_wstring(e.what())),
				MB_ICONERROR);
			m_taskbarProgr.clear();
			EndDialog(hwnd(), IDCANCEL);
		});
		return;
	}

	++m_filesDone;
	run_thread_ui([&]() {
		m_prog.set_pos(m_filesDone);
		m_taskbarProgr.set_pos(m_filesDone, opts.files.size());
		m_lbl.set_text( str::format(L"%u of %u files finished...",
			m_filesDone, opts.files.size()) );
	});

	if (m_filesDone < opts.files.size()) { // more files to come
		process_next_file();
	} else { // finished all processing
		run_thread_ui([&]() {
			datetime fin;
			sysdlg::msgbox(this, L"Conversion finished",
				str::format(L"%u files processed in %.2f seconds.",
					opts.files.size(),
					static_cast<double>(fin.ms_diff_from(m_time0)) / 1000),
				MB_ICONINFORMATION);
			m_taskbarProgr.clear();
			EndDialog(hwnd(), IDOK); // finally close dialog
		});
	}
}