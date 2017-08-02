
#pragma once
#include <winlamb/dialog_main.h>
#include <winlamb/msg_command.h>
#include <winlamb/msg_notify.h>
#include <winlamb-more/msg_initmenupopup.h>
#include <winlamb-more/checkbox.h>
#include <winlamb-more/combo.h>
#include <winlamb-more/file_ini.h>
#include <winlamb-more/listview.h>
#include <winlamb-more/resizer.h>
#include <winlamb-more/progress_taskbar.h>
#include <winlamb-more/textbox.h>

class Dlg_Main final :
	public wl::dialog_main,
	public wl::msg_command,
	public wl::msg_notify,
	public wl::msg_initmenupopup
{
private:
	wl::file_ini         m_iniFile;
	wl::progress_taskbar m_taskbarProg;
	wl::resizer          m_resz;
	wl::listview         m_lstFiles;
	wl::textbox          m_txtDest;
	wl::combo            m_cmbCbr, m_cmbVbr, m_cmbFlac, m_cmbNumThreads;
	wl::checkbox         m_radMp3, m_radMp3Cbr, m_radMp3Vbr, m_radFlac, m_radWav;
	wl::checkbox         m_chkDelSrc;

public:
	Dlg_Main();

private:
	bool    preliminar_checks();
	DWORD   num_processors() const;
	bool    dest_folder_is_ok();
	bool    files_exist(std::vector<std::wstring>& files);
	INT_PTR update_counter(size_t newCount);
	void    file_to_list(const std::wstring& file);
};