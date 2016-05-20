
#pragma once
#include "../winlamb/dialog_main.h"
#include "../winlamb/msg_command.h"
#include "../winlamb/msg_dropfiles.h"
#include "../winlamb/msg_initmenupopup.h"
#include "../winlamb/msg_notify.h"
#include "../winutil/checkbox.h"
#include "../winutil/combobox.h"
#include "../winutil/file_ini.h"
#include "../winutil/listview.h"
#include "../winutil/resizer.h"
#include "../winutil/taskbar_progress.h"
#include "../winutil/textbox.h"

class dlg_main final : public winlamb::dialog_main,
	public winlamb::dialog_msg_command,
	public winlamb::dialog_msg_dropfiles,
	public winlamb::dialog_msg_initmenupopup,
	public winlamb::dialog_msg_notify
{
private:
	winutil::file_ini         _ini;
	winutil::taskbar_progress _taskBar;
	winutil::resizer          _resizer;
	winutil::listview         _lstFiles;
	winutil::textbox          _txtDest;
	winutil::combobox         _cmbCbr, _cmbVbr, _cmbFlac, _cmbNumThreads;
	winutil::checkbox         _radMp3, _radMp3Cbr, _radMp3Vbr, _radFlac, _radWav;
	winutil::checkbox         _chkDelSrc;
public:
	dlg_main();
private:
	bool    _dest_folder_is_ok();
	bool    _files_exist(std::vector<std::wstring>& files);
	LRESULT _update_counter(size_t newCount);
	void    _file_to_list(const std::wstring& file);
};