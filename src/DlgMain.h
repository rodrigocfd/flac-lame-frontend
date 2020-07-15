
#pragma once
#include "../winlamb/dialog_main.h"
#include "../winlamb/button.h"
#include "../winlamb/checkbox.h"
#include "../winlamb/combobox.h"
#include "../winlamb/file_ini.h"
#include "../winlamb/listview.h"
#include "../winlamb/radio_group.h"
#include "../winlamb/resizer.h"
#include "../winlamb/progress_taskbar.h"
#include "../winlamb/textbox.h"

class DlgMain final : public wl::dialog_main {
private:
	wl::file_ini         mIniFile;
	wl::progress_taskbar mTaskbarProg;
	wl::resizer          mLayoutResizer;
	wl::listview         mLstFiles;
	wl::textbox          mTxtDest;
	wl::combobox         mCmbCbr, mCmbVbr, mCmbFlac, mCmbNumThreads;
	wl::radio_group      mRadMp3FlacWav, mRadMp3Type;
	wl::checkbox         mChkDelSrc;
	wl::button           mBtnRun;

public:
	DlgMain();

private:
	void    messages();
	void    validateIni();
	void    validateDestFolder();
	void    validateFilesExist(const std::vector<std::wstring>& files);
	INT_PTR updateRunBtnCounter(size_t newCount);
	void    putFileIntoList(const std::wstring& file);

	static DWORD numProcessors();
};