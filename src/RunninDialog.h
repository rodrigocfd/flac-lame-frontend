
#pragma once
#include "../toolow/toolow.h"
#include "../res/resource.h"

class RunninDialog final : public DialogModal {
public:
	enum class Target { NONE=0, MP3, FLAC, WAV };
private:
	Window               m_lbl;
	ProgressBar          m_prog;
	int                  m_numThreads;
	Target               m_targetType;
	const Array<String>& m_files;
	bool                 m_delSrc;
	bool                 m_isVbr;
	const String&        m_quality;
	const File::Ini&     m_ini;
	const String&        m_destFolder;
	int                  m_curFile, m_filesDone;
	System::Date         m_time0;
public:
	RunninDialog(
		int                  numThreads,
		Target               targetType,
		const Array<String>& files,
		bool                 delSrc,
		bool                 isVbr,
		const String&        quality,
		const File::Ini&     ini,
		const String&        destFolder );
	
	int show(Window *parent) { return DialogModal::show(parent, DLG_RUNNIN); }
private:
	INT_PTR msgHandler(UINT msg, WPARAM wp, LPARAM lp);
	void onInitDialog();
	void doProcessNextFile();
};