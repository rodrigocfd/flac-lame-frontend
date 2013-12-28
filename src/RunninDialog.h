
#pragma once
#include "../toolow/Dialog.h"
#include "../toolow/Controls.h"
#include "../toolow/File.h"
#include "../toolow/Date.h"
#include "../res/resource.h"

class RunninDialog : public DialogModal {
public:
	enum class Target { NONE=0, MP3, FLAC, WAV };

	RunninDialog(
		int            numThreads,
		Target         targetType,
		Array<String> *pFiles,
		bool           delSrc,
		bool           isVbr,
		const wchar_t *quality,
		File::Ini     *pIni,
		String        *pDestFolder );
	
	int show(Window *parent) { return DialogModal::show(parent, DLG_RUNNIN); }

private:
	INT_PTR msgHandler(UINT msg, WPARAM wp, LPARAM lp);
	void on_initDialog();
	void on_fileDone(LPARAM lp);

	Window         m_lbl;
	ProgressBar    m_prog;
	int            m_numThreads;
	Target         m_targetType;
	Array<String> *m_pFiles;
	bool           m_delSrc;
	bool           m_isVbr;
	wchar_t        m_quality[8];
	File::Ini     *m_pIni;
	String        *m_pDestFolder;
	int            m_filesDone;
	Date           m_time0;
};