
#pragma once
#include "../owl/owl.h"
using namespace owl;
using std::wstring;
using std::vector;

class RunninDialog final : public DialogModal {
public:
	enum class Target { NONE=0, MP3, FLAC, WAV };
private:
	Window                 m_lbl;
	ProgressBar            m_prog;
	int                    m_numThreads;
	Target                 m_targetType;
	const vector<wstring>& m_files;
	bool                   m_delSrc;
	bool                   m_isVbr;
	const wstring&         m_quality;
	const File::Ini&       m_ini;
	const wstring&         m_destFolder;
	int                    m_curFile, m_filesDone;
	Date                   m_time0;
public:
	RunninDialog(
		int                    numThreads,
		Target                 targetType,
		const vector<wstring>& files,
		bool                   delSrc,
		bool                   isVbr,
		const wstring&         quality,
		const File::Ini&       ini,
		const wstring&         destFolder );
private:
	void onInitDialog();
	void doProcessNextFile();
	
	INT_PTR dlgProc(UINT msg, WPARAM wp, LPARAM lp) {
		switch (msg) {
		case WM_INITDIALOG: this->onInitDialog(); break;
		}
		return DialogModal::dlgProc(msg, wp, lp);
	}
};