
#include "Dlg_Runnin.h"
#include "Convert.h"
#include "../winlamb/str.h"
#include "../winlamb/sys.h"
#include "../winlamb/sysdlg.h"
#include "../res/resource.h"
using namespace wl;
using std::wstring;
using std::vector;

Dlg_Runnin::Dlg_Runnin(
	progress_taskbar&      taskBar,
	int                    numThreads,
	target                 targetType,
	const vector<wstring>& files,
	bool                   delSrc,
	bool                   isVbr,
	const wstring&         quality,
	const file_ini&        ini,
	const wstring&         destFolder
)
	: m_taskbarProgr(taskBar), m_numThreads(numThreads), m_targetType(targetType),
		m_files(files), m_delSrc(delSrc), m_isVbr(isVbr), m_quality(quality), m_ini(ini),
		m_destFolder(destFolder), m_curFile(0), m_filesDone(0)
{
	setup.dialogId = DLG_RUNNIN;

	on_message(WM_INITDIALOG, [&](params&)
	{
		m_lbl.be(this, LBL_STATUS);
		m_prog.be(this, PRO_STATUS);

		m_prog.set_range(0, m_files.size());
		m_taskbarProgr.set_pos(0);
		m_lbl.set_text( str::format(L"0 of %d files finished...", m_files.size()) ); // initial text
		m_time0.set_now(); // start timer
		enable_x_button(false);

		// Proceed to the file conversion straight away.
		int nFiles = static_cast<int>(m_files.size());
		int batchSz = (m_numThreads < nFiles) ? m_numThreads : nFiles; // limit parallel processing
		for (int i = 0; i < batchSz; ++i) {
			sys::thread([&]() {
				process_next_file();
			});
		}

		center_on_parent();
		return TRUE;
	});
}

void Dlg_Runnin::process_next_file()
{
	int index = m_curFile++;
	if (index >= static_cast<int>(m_files.size())) return;

	const wstring& file = m_files[index];
	bool good = true;
	wstring err;

	switch (m_targetType) {
	case target::MP3:
		good = Convert::to_mp3(m_ini, file, m_destFolder, m_delSrc, m_quality, m_isVbr, &err);
		break;
	case target::FLAC:
		good = Convert::to_flac(m_ini, file, m_destFolder, m_delSrc, m_quality, &err);
		break;
	case target::WAV:
		good = Convert::to_wav(m_ini, file, m_destFolder, m_delSrc, &err);
	}

	if (!good) {
		m_curFile = static_cast<int>(m_files.size()); // error, so avoid further processing
		ui_thread([&]() {
			sysdlg::msgbox(this, L"Conversion failed",
				str::format(L"File #%d:\n%s\n%s", index, file.c_str(), err.c_str()),
				MB_ICONERROR);
			m_taskbarProgr.clear();
			EndDialog(hwnd(), IDCANCEL);
		});
	} else {
		++m_filesDone;
		ui_thread([&]() {
			m_prog.set_pos(m_filesDone);
			m_taskbarProgr.set_pos(m_filesDone, m_files.size());
			m_lbl.set_text( str::format(L"%d of %d files finished...",
				m_filesDone, m_files.size()) );
		});

		if (m_filesDone < static_cast<int>(m_files.size())) { // more files to come
			process_next_file();
		} else { // finished all processing
			ui_thread([&]() {
				datetime fin;
				sysdlg::msgbox(this, L"Conversion finished",
					str::format(L"%d files processed in %.2f seconds.",
						m_files.size(), static_cast<double>(fin.minus(m_time0)) / 1000),
					MB_ICONINFORMATION);
				m_taskbarProgr.clear();
				EndDialog(hwnd(), IDOK);
			});
		}
	}
}

void Dlg_Runnin::enable_x_button(bool enable) const
{
	// Enable/disable the X button to close the window; has no effect on Alt+F4.
	HMENU hMenu = GetSystemMenu(hwnd(), FALSE);
	if (hMenu) {
		UINT dwExtra = enable ? MF_ENABLED : (MF_DISABLED | MF_GRAYED);
		EnableMenuItem(hMenu, SC_CLOSE, MF_BYCOMMAND | dwExtra);
	}
}