
#pragma once
#include <winlamb/dialog_main.h>
#include <winlamb/button.h>
#include <winlamb/checkbox.h>
#include <winlamb/combobox.h>
#include <winlamb/file_ini.h>
#include <winlamb/listview.h>
#include <winlamb/radio_group.h>
#include <winlamb/resizer.h>
#include <winlamb/progress_taskbar.h>
#include <winlamb/textbox.h>

class Dlg_Main final : public wl::dialog_main {
private:
	wl::file_ini         m_iniFile;
	wl::progress_taskbar m_taskbarProg;
	wl::resizer          m_resz;
	wl::listview         m_lstFiles;
	wl::textbox          m_txtDest;
	wl::combobox         m_cmbCbr, m_cmbVbr, m_cmbFlac, m_cmbNumThreads;
	wl::radio_group      m_radMFW, m_radMp3Type;
	wl::checkbox         m_chkDelSrc;
	wl::button           m_btnRun;

public:
	Dlg_Main();

private:
	void    messages();
	void    validate_ini();
	void    validate_dest_folder();
	void    validate_files_exist(const std::vector<std::wstring>& files);
	INT_PTR update_counter(size_t newCount);
	void    file_to_list(const std::wstring& file);

	static DWORD num_processors();
};