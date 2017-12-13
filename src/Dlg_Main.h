
#pragma once
#include "std.h"
#include <winlamb/dialog_main.h>
#include <winlamb/button.h>
#include <winlamb/checkbox.h>
#include <winlamb/combobox.h>
#include <winlamb/file_ini.h>
#include <winlamb/listview.h>
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
	wl::checkbox         m_radMp3, m_radMp3Cbr, m_radMp3Vbr, m_radFlac, m_radWav;
	wl::checkbox         m_chkDelSrc;
	wl::button           m_btnRun;

public:
	Dlg_Main();

private:
	void    messages();
	void    validate_ini();
	void    validate_dest_folder();
	void    validate_files_exist(const vector<wstring>& files);
	INT_PTR update_counter(size_t newCount);
	void    file_to_list(const wstring& file);
};