
#include "RunninDialog.h"
#include "Converter.h"
#include "../toolow/util.h"

RunninDialog::RunninDialog(
	int            numThreads,
	Target::Type   targetType,
	Array<String> *pFiles,
	bool           delSrc,
	bool           isVbr,
	const wchar_t *quality,
	File::Ini     *pIni,
	const wchar_t *pDestFolder)
{
	m_numThreads = numThreads;debug(L"THREADS: %d\n", m_numThreads);
	m_targetType = targetType;
	m_pFiles = pFiles;
	m_delSrc = delSrc;
	m_isVbr = isVbr;
	lstrcpy(m_quality, quality);
	m_pIni = pIni;
	m_destFolder = pDestFolder;
	m_filesDone = 0; // incremented after each processing
}

INT_PTR RunninDialog::msgHandler(UINT msg, WPARAM wp, LPARAM lp)
{
	switch(msg)
	{
	case WM_INITDIALOG:          on_initDialog(); break;
	case Converter::WM_FILEDONE: on_fileDone(lp); return TRUE;
	}
	return DialogModal::msgHandler(msg, wp, lp);
}

void RunninDialog::on_initDialog()
{
	m_lbl = this->getChild(LBL_STATUS);
	m_prog = this->getChild(PRO_STATUS);
	
	m_prog.setRange(0, m_pFiles->size());
	m_lbl.setText(fmt(L"0 of %d files finished...", m_pFiles->size())->str());
	
	// Proceed to the file conversion straight away.
	int firstRun = (m_numThreads < m_pFiles->size()) ? m_numThreads : m_pFiles->size(); // limit parallel processing
	Array<Thread*> convs(firstRun);
	for(int i = 0; i < firstRun; ++i) {
		const wchar_t *file = (*m_pFiles)[i].str();
		switch(m_targetType) {
			case Target::MP3:
				convs[i] = new ConverterMp3(this->hWnd(), m_pIni, file, m_delSrc, m_quality, m_isVbr, m_destFolder.str());
				break;
			case Target::FLAC:
				convs[i] = new ConverterFlac(this->hWnd(), m_pIni, file, m_delSrc, m_quality, m_destFolder.str());
				break;
			case Target::WAV:
				convs[i] = new ConverterWav(this->hWnd(), m_pIni, file, m_delSrc, m_destFolder.str());
		}
	}

	this->setXButton(false);
	m_time0.setNow();
	Thread::RunParallelAsync(&convs); // notifications will be caught on fileDone()
}

void RunninDialog::on_fileDone(LPARAM lp)
{
	++m_filesDone;
	m_prog.setPos(m_filesDone);
	m_lbl.setText(fmt(L"%d of %d files finished...", m_filesDone, m_pFiles->size())->str());

	if(m_filesDone >= m_pFiles->size()) { // all files have been processed
		Date fin;
		this->messageBox(L"Conversion finished",
			fmt(L"%d files processed in %.2f seconds.", m_pFiles->size(), (double)fin.minus(m_time0) / 1000)->str(),
			MB_ICONINFORMATION);
		this->sendMessage(WM_CLOSE, 0, 0);
	}

	Thread *conv = NULL;
	const wchar_t *file = (*m_pFiles)[m_numThreads + m_filesDone - 1].str();
	switch(m_targetType) {
		case Target::MP3:
			conv = new ConverterMp3(this->hWnd(), m_pIni, file, m_delSrc, m_quality, m_isVbr, m_destFolder.str());
			break;
		case Target::FLAC:
			conv = new ConverterFlac(this->hWnd(), m_pIni, file, m_delSrc, m_quality, m_destFolder.str());
			break;
		case Target::WAV:
			conv = new ConverterWav(this->hWnd(), m_pIni, file, m_delSrc, m_destFolder.str());
	}
	conv->runAsync();
}