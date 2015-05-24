
#include "RunninDialog.h"
#include "Convert.h"
#include "../res/resource.h"

RunninDialog::RunninDialog(
	int                    numThreads,
	Target                 targetType,
	const vector<wstring>& files,
	bool                   delSrc,
	bool                   isVbr,
	const wstring&         quality,
	const file::Ini&       ini,
	const wstring&         destFolder )
	: DialogModal(DLG_RUNNIN),
		m_numThreads(numThreads), m_targetType(targetType), m_files(files), m_delSrc(delSrc), m_isVbr(isVbr),
		m_quality(quality), m_ini(ini), m_destFolder(destFolder), m_curFile(0), m_filesDone(0)
{
	// m_curFile and m_filesDone are incremented after each processing.
}

void RunninDialog::onInitDialog()
{
	m_lbl = this->getChild(LBL_STATUS);
	m_prog = this->getChild(PRO_STATUS);
	
	m_prog.setRange(0, m_files.size());
	m_lbl.setText( str::Sprintf(L"0 of %d files finished...", m_files.size()) ); // initial text
	m_time0.setNow(); // start timer
	this->setXButton(false);
	
	// Proceed to the file conversion straight away.
	int nFiles = static_cast<int>(m_files.size());
	int batchSz = (m_numThreads < nFiles) ? m_numThreads : nFiles; // limit parallel processing
	for (int i = 0; i < batchSz; ++i) {
		sys::Thread([&]() {
			this->doProcessNextFile();
		});
	}
}

void RunninDialog::doProcessNextFile()
{
	int index = m_curFile++;
	if (index >= static_cast<int>(m_files.size())) return;

	const wstring& file = m_files[index];
	bool good = true;
	wstring err;

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
		m_curFile = static_cast<int>(m_files.size()); // error, so avoid further processing
		this->sendFunction([&]() {
			this->messageBox(L"Conversion failed",
				str::Sprintf(L"File #%d:\n%s\n%s", index, file.c_str(), err.c_str()),
				MB_ICONERROR);
			this->endDialog(IDCANCEL);
		});
	} else {
		++m_filesDone;

		this->sendFunction([&]() {
			m_prog.setPos(m_filesDone);
			m_lbl.setText( str::Sprintf(L"%d of %d files finished...", m_filesDone, m_files.size()) );
		});
			
		if (m_filesDone < static_cast<int>(m_files.size())) { // more files to come
			this->doProcessNextFile();
		} else { // finished all processing
			this->sendFunction([&]() {
				Date fin;
				this->messageBox(L"Conversion finished",
					str::Sprintf(L"%d files processed in %.2f seconds.",
						m_files.size(), static_cast<double>(fin.minus(m_time0)) / 1000),
					MB_ICONINFORMATION);
				this->endDialog(IDOK);
			});
		}
	}
}