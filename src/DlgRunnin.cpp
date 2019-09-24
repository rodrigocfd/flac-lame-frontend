
#include "DlgRunnin.h"
#include <winlamb/str.h>
#include <winlamb/sysdlg.h>
#include "Convert.h"
#include "../res/resource.h"
using std::wstring;
using namespace wl;

DlgRunnin::DlgRunnin(progress_taskbar& taskbarProgr, const file_ini& iniFile)
	: mTaskbarProgr(taskbarProgr), mIniFile(iniFile)
{
	setup.dialogId = DLG_RUNNIN;

	on_message(WM_INITDIALOG, [&](params)
	{
		mLbl.assign(this, LBL_STATUS);
		mProg.assign(this, PRO_STATUS);

		mProg.set_range(0, opts.files.size());
		mTaskbarProgr.set_pos(0);
		mLbl.set_text( str::format(L"0 of %u files finished...", opts.files.size()) ); // initial text
		mTime0.set_now(); // start timer

		// Proceed to the file conversion straight away.
		size_t batchSz = (opts.numThreads < opts.files.size()) ?
			opts.numThreads : opts.files.size(); // limit parallel processing

		for (size_t i = 0; i < batchSz; ++i) {
			run_thread_detached([&]() {
				processNextFile();
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

void DlgRunnin::processNextFile()
{
	size_t curIndex = mCurFile++;
	if (curIndex >= opts.files.size()) return;

	const wstring& file = opts.files[curIndex];

	try {
		switch (opts.targetType) {
		case target::MP3:
			Convert::toMp3(mIniFile, file, opts.destFolder, opts.delSrc, opts.quality, opts.isVbr);
			break;
		case target::FLAC:
			Convert::toFlac(mIniFile, file, opts.destFolder, opts.delSrc, opts.quality);
			break;
		case target::WAV:
			Convert::toWav(mIniFile, file, opts.destFolder, opts.delSrc);
		}
	} catch (const std::exception& e) {
		mCurFile = opts.files.size(); // error, so avoid further processing
		run_thread_ui([&]() {
			sysdlg::msgbox(this, L"Conversion failed",
				str::format(L"File #%u:\n%s\n%s",
					curIndex, file, str::to_wstring(e.what())),
				MB_ICONERROR);
			mTaskbarProgr.clear();
			EndDialog(hwnd(), IDCANCEL);
		});
		return;
	}

	++mFilesDone;
	run_thread_ui([&]() {
		mProg.set_pos(mFilesDone);
		mTaskbarProgr.set_pos(mFilesDone, opts.files.size());
		mLbl.set_text( str::format(L"%u of %u files finished...",
			mFilesDone, opts.files.size()) );
	});

	if (mFilesDone < opts.files.size()) { // more files to come
		processNextFile();
	} else { // finished all processing
		run_thread_ui([&]() {
			datetime fin;
			sysdlg::msgbox(this, L"Conversion finished",
				str::format(L"%u files processed in %.2f seconds.",
					opts.files.size(),
					static_cast<double>(fin.ms_diff_from(mTime0)) / 1000),
				MB_ICONINFORMATION);
			mTaskbarProgr.clear();
			EndDialog(hwnd(), IDOK); // finally close dialog
		});
	}
}