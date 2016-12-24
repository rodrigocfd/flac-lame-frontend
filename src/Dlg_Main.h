
#pragma once
#include "../wet/dialog_main.h"
#include "../wet/checkbox.h"
#include "../wet/combo.h"
#include "../wet/file_ini.h"
#include "../wet/listview.h"
#include "../wet/resizer.h"
#include "../wet/progress_taskbar.h"
#include "../wet/textbox.h"

class Dlg_Main final : public wet::dialog_main {
private:
	wet::file_ini         _ini;
	wet::progress_taskbar _taskBar;
	wet::resizer          _resizer;
	wet::listview         _lstFiles;
	wet::textbox          _txtDest;
	wet::combo            _cmbCbr, _cmbVbr, _cmbFlac, _cmbNumThreads;
	wet::checkbox         _radMp3, _radMp3Cbr, _radMp3Vbr, _radFlac, _radWav;
	wet::checkbox         _chkDelSrc;

public:
	Dlg_Main();

private:
	INT_PTR  proc(wet::params p) override;
	bool    _preliminar_checks();
	bool    _dest_folder_is_ok();
	bool    _files_exist(std::vector<std::wstring>& files);
	LRESULT _update_counter(size_t newCount);
	void    _file_to_list(const std::wstring& file);
};