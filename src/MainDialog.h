
#pragma once
#include "../toolow/Dialog.h"
#include "../toolow/Resizer.h"
#include "../toolow/ListView.h"
#include "../toolow/Controls.h"
#include "../toolow/File.h"

class MainDialog : public DialogApp {
private:
	INT_PTR msgHandler(UINT msg, WPARAM wp, LPARAM lp);
	void on_initDialog();
	void on_esc();
	void on_dropFiles(WPARAM wp);
	void on_chooseDest();
	void on_selectFormat();
	void on_selectRate();
	void on_run();
	void on_fileDone(LPARAM lp);

	void do_fileToList(const wchar_t *file);
	void do_updateCounter(int newCount);

	File::Ini m_ini;
	Resizer   m_resizer;
	ListView  m_lstFiles;
	Combo     m_cmbCbr, m_cmbVbr, m_cmbFlac, m_cmbNumThreads;
	Radio     m_radMp3, m_radMp3Cbr, m_radMp3Vbr, m_radFlac, m_radWav;
	CheckBox  m_chkDelSrc;
};