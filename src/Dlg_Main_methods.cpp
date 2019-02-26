
#include "Dlg_Main.h"
#include <winlamb/executable.h>
#include <winlamb/path.h>
#include <winlamb/sysdlg.h>
#include "Convert.h"
#include "../res/resource.h"
using std::vector;
using std::wstring;
using namespace wl;

RUN(Dlg_Main);

Dlg_Main::Dlg_Main()
{
	setup.dialogId = DLG_MAIN;
	setup.iconId = ICO_MAIN;
	setup.accelTableId = ACC_MAIN;

	messages();
}

void Dlg_Main::validate_ini()
{
	// Validate and load INI file.
	wstring iniPath = executable::get_own_path().append(L"\\flac-lame-frontend.ini");
	if (!file::util::exists(iniPath)) {
		throw std::runtime_error(str::to_ascii(
			str::format(L"File not found:\n%s", iniPath) ));
	}

	m_iniFile.load_from_file(iniPath);
	Convert::validate_paths(m_iniFile); // validate tools
}

void Dlg_Main::validate_dest_folder()
{
	wstring destFolder = m_txtDest.get_text();
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

void Dlg_Main::validate_files_exist(const vector<wstring>& files)
{
	for (const wstring& f : files) { // each filepath
		if (!file::util::exists(f)) {
			throw std::runtime_error(str::to_ascii(
				str::format(L"Process aborted, file does not exist:\n%s", f) ));
		}
	}
}

INT_PTR Dlg_Main::update_counter(size_t newCount)
{
	// Update counter on Run button.
	wstring caption = newCount ?
		str::format(L"&Run (%d)", newCount) : L"&Run";
	m_btnRun.set_text(caption);
	EnableWindow(m_btnRun.hwnd(), newCount > 0); // Run button enabled if at least 1 file
	return 0;
};

void Dlg_Main::file_to_list(const wstring& file)
{
	int iType = -1;
	if (path::has_extension(file, L".mp3"))       iType = 0;
	else if (path::has_extension(file, L".flac")) iType = 1;
	else if (path::has_extension(file, L".wav"))  iType = 2; // what type of audio file is this?

	if (iType == -1) {
		return; // bypass file if unaccepted format
	}
	if (!m_lstFiles.items.exists(file)) {
		m_lstFiles.items.add(file, iType); // add only if not present yet
	}
}

DWORD Dlg_Main::num_processors()
{
	SYSTEM_INFO si{};
	GetSystemInfo(&si);
	return si.dwNumberOfProcessors;
}