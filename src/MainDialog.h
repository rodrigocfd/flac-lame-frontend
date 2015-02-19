
#pragma once
#include "../owl/owl.h"
#include "../res/resource.h"
using namespace owl;
using std::wstring;

class MainDialog final : public DialogApp {
private:
	File::Ini m_ini;
	Resizer   m_resizer;
	ListView  m_lstFiles;
	Combo     m_cmbCbr, m_cmbVbr, m_cmbFlac, m_cmbNumThreads;
	Radio     m_radMp3, m_radMp3Cbr, m_radMp3Vbr, m_radFlac, m_radWav;
	CheckBox  m_chkDelSrc;
public:
	MainDialog() : DialogApp(DLG_MAIN, ICO_MAIN) { }
private:
	void onInitDialog();
	void onDropFiles(WPARAM wp);
	void onEsc();
	void onChooseDest();
	void onSelectFormat();
	void onSelectRate();
	void onRun();
	void onFileDone(LPARAM lp);

	void doFileToList(const wstring& file);
	void doUpdateCounter(int newCount);	

	INT_PTR dlgProc(UINT msg, WPARAM wp, LPARAM lp) {
		switch (msg) {
		case WM_INITDIALOG: this->onInitDialog(); break;
		case WM_DROPFILES:  this->onDropFiles(wp); return TRUE;
		case WM_COMMAND:
			switch (LOWORD(wp)) {
			case IDCANCEL: this->onEsc(); return TRUE; // close on ESC
			case BTN_DEST: this->onChooseDest(); return TRUE;
			case RAD_MP3:
			case RAD_FLAC:
			case RAD_WAV:  this->onSelectFormat(); return TRUE;
			case RAD_CBR:
			case RAD_VBR:  this->onSelectRate(); return TRUE;
			case BTN_RUN:  this->onRun(); return TRUE;
			}
			break;
		case WM_NOTIFY:
			switch ((reinterpret_cast<NMHDR*>(lp))->idFrom) {
			case LST_FILES:
				switch ((reinterpret_cast<NMHDR*>(lp))->code) {
				case LVN_INSERTITEM:     this->doUpdateCounter(m_lstFiles.items.count()); return TRUE; // new item inserted
				case LVN_DELETEITEM:     this->doUpdateCounter(m_lstFiles.items.count() - 1); return TRUE; // item about to be deleted
				case LVN_DELETEALLITEMS: this->doUpdateCounter(0); return TRUE; // all items about to be deleted
				case LVN_KEYDOWN:
					switch ((reinterpret_cast<NMLVKEYDOWN*>(lp))->wVKey) {
					case VK_DELETE: m_lstFiles.items.removeSelected(); return TRUE; // Del key
					}
					break;
				}
				break;
			}
			break;
		}
		return DialogApp::dlgProc(msg, wp, lp);
	}
};