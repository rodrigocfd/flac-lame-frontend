
#include "DlgMain.h"
#include <winlamb/executable.h>
#include <winlamb/path.h>
#include <winlamb/sysdlg.h>
#include "Convert.h"
#include "../res/resource.h"
using std::vector;
using std::wstring;
using namespace wl;

RUN(DlgMain);

DlgMain::DlgMain()
{
	setup.dialogId = DLG_MAIN;
	setup.iconId = ICO_MAIN;
	setup.accelTableId = ACC_MAIN;

	messages();
}

void DlgMain::validateIni()
{
	// Validate and load INI file.
	wstring iniPath = executable::get_own_path().append(L"\\flac-lame-frontend.ini");
	if (!file::util::exists(iniPath)) {
		throw std::runtime_error(str::to_ascii(
			str::format(L"File not found:\n%s", iniPath) ));
	}

	mIniFile.load_from_file(iniPath);
	Convert::validatePaths(mIniFile); // validate tools
}

void DlgMain::validateDestFolder()
{
	wstring destFolder = mTxtDest.get_text();
	if (destFolder.empty()) return;

	if (!file::util::exists(destFolder)) {
		int q = sysdlg::msgbox(this, L"Create directory",
			str::format(L"The following directory:\n%s\ndoes not exist. Create it?",
				destFolder),
			MB_ICONQUESTION | MB_YESNO);

		if (q == IDYES) {
			try {
				file::util::create_dir(destFolder);
			} catch (const std::exception& e) {
				throw std::runtime_error(str::to_ascii(
					str::format(L"The directory failed to be created:\n%s\n%s",
						destFolder, str::to_wstring(e.what())) ));
			}
		} else { // user didn't want to create the new dir
			return; // halt
		}
	} else if (!file::util::is_dir(destFolder)) {
		throw std::runtime_error(str::to_ascii(
			str::format(L"The following path is not a directory:\n%s", destFolder) ));
	}
}

void DlgMain::validateFilesExist(const vector<wstring>& files)
{
	for (const wstring& f : files) { // each filepath
		if (!file::util::exists(f)) {
			throw std::runtime_error(str::to_ascii(
				str::format(L"Process aborted, file does not exist:\n%s", f) ));
		}
	}
}

INT_PTR DlgMain::updateRunBtnCounter(size_t newCount)
{
	wstring caption = newCount ?
		str::format(L"&Run (%d)", newCount) : L"&Run";
	mBtnRun.set_text(caption)
		.set_enabled(newCount > 0); // Run button enabled if at least 1 file
	return 0;
};

void DlgMain::putFileIntoList(const wstring& file)
{
	int iType = -1;
	if (path::has_extension(file, L".mp3"))       iType = 0;
	else if (path::has_extension(file, L".flac")) iType = 1;
	else if (path::has_extension(file, L".wav"))  iType = 2; // what type of audio file is this?

	if (iType == -1) {
		return; // bypass file if unaccepted format
	}
	if (!mLstFiles.items.exists(file)) {
		mLstFiles.items.add_with_icon(file, iType); // add only if not present yet
	}
}

DWORD DlgMain::numProcessors()
{
	SYSTEM_INFO si{};
	GetSystemInfo(&si);
	return si.dwNumberOfProcessors;
}