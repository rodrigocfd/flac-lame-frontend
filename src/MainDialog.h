
#pragma once
#include "../toolow/toolow.h"
#include "../res/resource.h"

class MainDialog final : public DialogApp {
private:
	File::Ini m_ini;
	Resizer   m_resizer;
	ListView  m_lstFiles;
	Combo     m_cmbCbr, m_cmbVbr, m_cmbFlac, m_cmbNumThreads;
	Radio     m_radMp3, m_radMp3Cbr, m_radMp3Vbr, m_radFlac, m_radWav;
	CheckBox  m_chkDelSrc;
public:
	int run(HINSTANCE hInst, LPWSTR cmdLine, int cmdShow) { return DialogApp::run(hInst, cmdShow, DLG_MAIN, ICO_MAIN); }
private:
	INT_PTR msgHandler(UINT msg, WPARAM wp, LPARAM lp);
	void onInitDialog();
	void onEsc();
	void onDropFiles(WPARAM wp);
	void onChooseDest();
	void onSelectFormat();
	void onSelectRate();
	void onRun();
	void onFileDone(LPARAM lp);

	void doFileToList(const String& file);
	void doUpdateCounter(int newCount);	
};