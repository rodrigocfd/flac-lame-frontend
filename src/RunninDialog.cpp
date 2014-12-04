
#include "RunninDialog.h"
#include "Convert.h"

RunninDialog::RunninDialog(
	int                  numThreads,
	Target               targetType,
	const Array<String>& files,
	bool                 delSrc,
	bool                 isVbr,
	const String&        quality,
	const File::Ini&     ini,
	const String&        destFolder )
	: m_numThreads(numThreads), m_targetType(targetType), m_files(files), m_delSrc(delSrc), m_isVbr(isVbr),
		m_quality(quality), m_ini(ini), m_destFolder(destFolder), m_curFile(0), m_filesDone(0)
{
	// m_curFile and m_filesDone are incremented after each processing.
}

INT_PTR RunninDialog::msgHandler(UINT msg, WPARAM wp, LPARAM lp)
{
	switch (msg)
	{
	case WM_INITDIALOG: this->onInitDialog(); break;
	}
	return DialogModal::msgHandler(msg, wp, lp);
}

void RunninDialog::onInitDialog()
{
	m_lbl = this->getChild(LBL_STATUS);
	m_prog = this->getChild(PRO_STATUS);
	
	m_prog.setRange(0, m_files.size());
	m_lbl.setText( String::Fmt(L"0 of %d files finished...", m_files.size()) ); // initial text
	m_time0.setNow(); // start timer
	this->setXButton(false);
	
	// Proceed to the file conversion straight away.
	int batchSz = (m_numThreads < m_files.size()) ? m_numThreads : m_files.size(); // limit parallel processing
	for (int i = 0; i < batchSz; ++i)
		System::Thread([=]() { this->doProcessNextFile(); });
}

void RunninDialog::doProcessNextFile()
{
	int index = m_curFile++;
	if (index >= m_files.size()) return;

	const String& file = m_files[index];
	bool good = true;
	String err;

	switch (m_targetType) {
	case Target::MP3:
		good = Convert::ToMp3(m_ini, file, m_destFolder, m_delSrc, m_quality, m_isVbr, &err);
		break;
	case Target::FLAC:
		good = Convert::ToFlac(m_ini, file, m_destFolder, m_delSrc, m_quality, &err);
		break;
	case Target::WAV:
		good = Convert::ToWav(m_ini, file, m_destFolder, m_delSrc, &err);
	}

	if (!good) {
		m_curFile = m_files.size(); // error, so avoid further processing
		this->sendFunction([=]() {
			this->messageBox(L"Conversion failed",
				String::Fmt(L"File #%d:\n%s\n%s", index, file.str(), err.str()),
				MB_ICONERROR);
			this->endDialog(IDCANCEL);
		});
	} else {
		++m_filesDone;

		this->sendFunction([=]() { // update GUI
			m_prog.setPos(m_filesDone);
			m_lbl.setText( String::Fmt(L"%d of %d files finished...", m_filesDone, m_files.size()) );
		});
			
		if (m_filesDone < m_files.size()) { // more files to come
			this->doProcessNextFile();
		} else { // finished all processing
			this->sendFunction([=]() {
				Date fin;
				this->messageBox(L"Conversion finished",
					String::Fmt(L"%d files processed in %.2f seconds.", m_files.size(), (double)fin.minus(m_time0) / 1000),
					MB_ICONINFORMATION);
				this->endDialog(IDOK);
			});
		}
	}
}