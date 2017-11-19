
#include "Dlg_Runnin.h"
#include <winlamb/str.h>
#include <winlamb/sysdlg.h>
#include <winlamb/thread.h>
#include "Convert.h"
#include "res/resource.h"
using namespace wl;

Dlg_Runnin::Dlg_Runnin(
	progress_taskbar&      taskBar,
	size_t                 numThreads,
	target                 targetType,
	const vector<wstring>& files,
	bool                   delSrc,
	bool                   isVbr,
	wstring                quality,
	const file_ini&        ini,
	wstring                destFolder
)
	: m_taskbarProgr(taskBar), m_numThreads(numThreads), m_targetType(targetType),
		m_files(files), m_delSrc(delSrc), m_isVbr(isVbr), m_quality(quality), m_ini(ini),
		m_destFolder(destFolder), m_curFile(0), m_filesDone(0)
{
	setup.dialogId = DLG_RUNNIN;

	on_message(WM_INITDIALOG, [&](wm::initdialog)
	{
		m_lbl.assign(this, LBL_STATUS);
		m_prog.assign(this, PRO_STATUS);

		m_prog.set_range(0, m_files.size());
		m_taskbarProgr.set_pos(0);
		m_lbl.set_text( str::format(L"0 of %u files finished...", m_files.size()) ); // initial text
		m_time0.set_now(); // start timer

		// Proceed to the file conversion straight away.
		size_t batchSz = (m_numThreads < m_files.size()) ?
			m_numThreads : m_files.size(); // limit parallel processing

		for (size_t i = 0; i < batchSz; ++i) {
			thread::run_detached([&]() {
				process_next_file();
			});
		}

		center_on_parent();
		return TRUE;
	});

	on_message(WM_CLOSE, [](wm::close)
	{
		return TRUE; // don't close the dialog, EndDialog() not called
	});
}

void Dlg_Runnin::process_next_file()
{
	size_t curIndex = m_curFile++;
	if (curIndex >= m_files.size()) return;

	const wstring& file = m_files[curIndex];

	try {
	switch (m_targetType) {
		case target::MP3:
			Convert::to_mp3(m_ini, file, m_destFolder, m_delSrc, m_quality, m_isVbr);
			break;
		case target::FLAC:
			Convert::to_flac(m_ini, file, m_destFolder, m_delSrc, m_quality);
			break;
		case target::WAV:
			Convert::to_wav(m_ini, file, m_destFolder, m_delSrc);
		}
	} catch (const std::exception& e) {
		m_curFile = m_files.size(); // error, so avoid further processing
		run_ui_thread([&]() {
			sysdlg::msgbox(this, L"Conversion failed",
				str::format(L"File #%u:\n%s\n%s",
					curIndex, file, str::parse_ascii(e.what())),
				MB_ICONERROR);
			m_taskbarProgr.clear();
			EndDialog(hwnd(), IDCANCEL);
		});
		return;
	}

	++m_filesDone;
	run_ui_thread([&]() {
		m_prog.set_pos(m_filesDone);
		m_taskbarProgr.set_pos(m_filesDone, m_files.size());
		m_lbl.set_text( str::format(L"%u of %u files finished...",
			m_filesDone, m_files.size()) );
	});

	if (m_filesDone < m_files.size()) { // more files to come
		process_next_file();
	} else { // finished all processing
		run_ui_thread([&]() {
			datetime fin;
			sysdlg::msgbox(this, L"Conversion finished",
				str::format(L"%u files processed in %.2f seconds.",
					m_files.size(), static_cast<double>(fin.minus(m_time0)) / 1000),
				MB_ICONINFORMATION);
			m_taskbarProgr.clear();
			EndDialog(hwnd(), IDOK); // finally close dialog
		});
	}
}