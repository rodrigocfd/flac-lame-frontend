
#pragma once
#include "../winlamb/dialog_main.h"
#include "../winlamb/msg_command.h"
#include "../winlamb/msg_initmenupopup.h"
#include "../winlamb/msg_notify.h"
#include "../winutil/CheckBox.h"
#include "../winutil/ComboBox.h"
#include "../winutil/FileIni.h"
#include "../winutil/ListView.h"
#include "../winutil/Resizer.h"
#include "../winutil/TaskBarProgress.h"
#include "../winutil/TextBox.h"

class DlgMain final : public winlamb::dialog_main,
	winlamb::msg_command_dialog,
	winlamb::msg_initmenupopup_dialog,
	winlamb::msg_notify_dialog
{
private:
	FileIni         _ini;
	TaskBarProgress _taskBar;
	Resizer         _resizer;
	ListView        _lstFiles;
	TextBox         _txtDest;
	ComboBox        _cmbCbr, _cmbVbr, _cmbFlac, _cmbNumThreads;
	CheckBox        _radMp3, _radMp3Cbr, _radMp3Vbr, _radFlac, _radWav;
	CheckBox        _chkDelSrc;
public:
	DlgMain();
private:
	bool    _destFolderIsOk();
	bool    _filesExist(std::vector<std::wstring>& files);
	LRESULT _doUpdateCounter(int newCount);
	void    _doFileToList(const std::wstring& file);
};