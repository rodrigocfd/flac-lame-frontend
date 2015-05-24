
#include <algorithm>
#include "MainDialog.h"
#include "RunninDialog.h"
#include "Convert.h"

RUN(MainDialog);

void MainDialog::onInitDialog()
{
	// Validate and load INI file.
	m_ini.setPath( sys::GetExePath().append(L"\\FlacLameFE.ini") );
	
	wstring err;
	if (!m_ini.load(&err)) {
		this->messageBox(L"Fail", err, MB_ICONERROR);
		this->sendMessage(WM_CLOSE, 0, 0); // halt program
		return;
	}

	// Validate tools.
	if (!Convert::PathsAreValid(m_ini, &err)) {
		this->messageBox(L"Fail", err, MB_ICONERROR);
		this->sendMessage(WM_CLOSE, 0, 0); // halt program
		return;
	}

	// Layout control when resizing.
	m_resizer.add({ LST_FILES }, this->hWnd(), Resizer::Do::RESIZE, Resizer::Do::RESIZE)
		.add({ TXT_DEST }, this->hWnd(), Resizer::Do::RESIZE, Resizer::Do::REPOS)
		.add({ LBL_DEST, FRA_CONV, RAD_MP3, RAD_FLAC, RAD_WAV, RAD_CBR, RAD_VBR, LBL_LEVEL,
			CMB_CBR, CMB_VBR, CMB_FLAC, CHK_DELSRC, LBL_NUMTHREADS, CMB_NUMTHREADS },
			this->hWnd(), Resizer::Do::NOTHING, Resizer::Do::REPOS)
		.add({ BTN_DEST, BTN_RUN }, this->hWnd(), Resizer::Do::REPOS, Resizer::Do::REPOS)
		.afterResize([&]() { m_lstFiles.columnFit(0); });

	// Main listview initialization.
	m_lstFiles = this->getChild(LST_FILES);
	m_lstFiles.columnAdd(L"File", 300)
		.columnFit(0)
		.iconPush(L"mp3")
		.iconPush(L"flac")
		.iconPush(L"wav"); // icons of the 3 filetypes we use

	// Initializing comboboxes.
	m_cmbCbr = this->getChild(CMB_CBR);
	m_cmbCbr.itemAdd({ L"32 kbps", L"40 kbps", L"48 kbps", L"56 kbps", L"64 kbps", L"80 kbps", L"96 kbps",
		L"112 kbps", L"128 kbps; default", L"160 kbps", L"192 kbps", L"224 kbps", L"256 kbps", L"320 kbps" });
	m_cmbCbr.itemSetSelected(8);

	m_cmbVbr = this->getChild(CMB_VBR);
	m_cmbVbr.itemAdd({ L"0 (~245 kbps)", L"1 (~225 kbps)", L"2 (~190 kbps)", L"3 (~175 kbps)",
		L"4 (~165 kbps); default", L"5 (~130 kbps)", L"6 (~115 kbps)", L"7 (~100 kbps)",
		L"8 (~85 kbps)", L"9 (~65 kbps)" });
	m_cmbVbr.itemSetSelected(4);

	m_cmbFlac = this->getChild(CMB_FLAC);
	m_cmbFlac.itemAdd({ L"1", L"2", L"3", L"4", L"5", L"6", L"7", L"8" });
	m_cmbFlac.itemSetSelected(7);

	m_cmbNumThreads = this->getChild(CMB_NUMTHREADS);
	m_cmbNumThreads.itemAdd({ L"1", L"2", L"4", L"8" });
	SYSTEM_INFO si = { 0 };
	GetSystemInfo(&si);
	switch (si.dwNumberOfProcessors) {
		case 1: m_cmbNumThreads.itemSetSelected(0); break;
		case 2: m_cmbNumThreads.itemSetSelected(1); break;
		case 4: m_cmbNumThreads.itemSetSelected(2); break;
		case 8: m_cmbNumThreads.itemSetSelected(3); break;
		default: m_cmbNumThreads.itemSetSelected(0);
	}

	// Initializing radio buttons.
	m_radMp3    = this->getChild(RAD_MP3); m_radMp3.setCheck(true, Radio::EmulateClick::YES);
	m_radMp3Cbr = this->getChild(RAD_CBR);
	m_radMp3Vbr = this->getChild(RAD_VBR); m_radMp3Vbr.setCheck(true, Radio::EmulateClick::YES);
	m_radFlac   = this->getChild(RAD_FLAC);
	m_radWav    = this->getChild(RAD_WAV);

	m_chkDelSrc = this->getChild(CHK_DELSRC);
}

void MainDialog::onDropFiles(WPARAM wp)
{
	for (const wstring& drop : this->getDroppedFiles(reinterpret_cast<HDROP>(wp))) {
		if (file::IsDir(drop)) { // if a directory, add all files inside of it
			for (const wstring f : file::Listing::GetAll(drop, L"*.mp3")) {
				this->doFileToList(f);
			}
			for (const wstring f : file::Listing::GetAll(drop, L"*.flac")) {
				this->doFileToList(f);
			}
			for (const wstring f : file::Listing::GetAll(drop, L"*.wav")) {
				this->doFileToList(f);
			}
		} else {
			this->doFileToList(drop); // add single file
		}
	}
	this->doUpdateCounter( m_lstFiles.items.count() );
}

void MainDialog::onEsc()
{
	if (!m_lstFiles.items.count() || this->getChild(BTN_RUN).isEnabled()) {
		this->sendMessage(WM_CLOSE, 0, 0); // close on ESC only if not processing
	}
}

void MainDialog::onChooseDest()
{
	wstring folder;
	if (this->getFolderChoose(folder)) {
		this->getChild(TXT_DEST).setText(folder.c_str()).setFocus();
	}
}

void MainDialog::onSelectFormat()
{
	m_radMp3Cbr.setEnable( m_radMp3.isChecked() );
	m_cmbCbr.setEnable( m_radMp3.isChecked() && m_radMp3Cbr.isChecked() );

	m_radMp3Vbr.setEnable( m_radMp3.isChecked() );
	m_cmbVbr.setEnable( m_radMp3.isChecked() && m_radMp3Vbr.isChecked() );

	this->getChild(LBL_LEVEL).setEnable( m_radFlac.isChecked() );
	m_cmbFlac.setEnable( m_radFlac.isChecked() );
}

void MainDialog::onSelectRate()
{
	m_cmbCbr.setEnable( m_radMp3Cbr.isChecked() );
	m_cmbVbr.setEnable( m_radMp3Vbr.isChecked() );
}

void MainDialog::onRun()
{
	// Validate destination folder, if any.
	wstring destFolder = this->getChild(TXT_DEST).getText();
	if (!destFolder.empty()) {
		if (!file::Exists(destFolder)) {
			int q = this->messageBox(L"Create directory",
				str::Sprintf(L"The following directory:\n%s\ndoes not exist. Create it?", destFolder.c_str()),
				MB_ICONQUESTION | MB_YESNO);
			if (q == IDYES) {
				if (!file::CreateDir(destFolder)) {
					this->messageBox(L"Fail",
						str::Sprintf(L"The directory failed to be created:\n%s", destFolder.c_str()),
						MB_ICONERROR);
					return; // halt
				}
			} else { // user didn't want to create the new dir
				return; // halt
			}
		} else if (!file::IsDir(destFolder)) {
			this->messageBox(L"Fail",
				str::Sprintf(L"The following path is not a directory:\n%s", destFolder.c_str()),
				MB_ICONERROR);
			return; // halt
		}
	}

	// Check the existence of each file added to list.
	vector<ListView::Item> allItems = m_lstFiles.items.getAll();
	vector<wstring> files(allItems.size());
	std::transform(allItems.begin(), allItems.end(), files.begin(),
		[](ListView::Item& item)->wstring { return item.getText(0); });
	for (const wstring& f : files) { // each filepath
		if (!file::Exists(f)) {
			this->messageBox(L"Fail",
				str::Sprintf(L"Process aborted, file does not exist:\n%s", f.c_str()),
				MB_ICONERROR);
			return; // halt
		}
	}

	// Retrieve settings.
	bool delSrc = m_chkDelSrc.isChecked();
	bool isVbr = m_radMp3Vbr.isChecked();
	int numThreads = std::stoi(m_cmbNumThreads.itemGetSelectedText());
	
	wstring quality;
	if (m_radMp3.isChecked()) {
		Combo& cmbQuality = (isVbr ? m_cmbVbr : m_cmbCbr);
		quality = cmbQuality.itemGetSelectedText();
		quality.resize(str::Find(str::Sens::YES, quality, L' ')); // first characters of chosen option are the quality setting itself
	} else if (m_radFlac.isChecked()) {
		quality = m_cmbFlac.itemGetSelectedText(); // text is quality setting itself
	}

	// Which format are we converting to?
	RunninDialog::Target targetType = RunninDialog::Target::NONE;
	
	if (m_radMp3.isChecked())       targetType = RunninDialog::Target::MP3;
	else if (m_radFlac.isChecked()) targetType = RunninDialog::Target::FLAC;
	else if (m_radWav.isChecked())  targetType = RunninDialog::Target::WAV;

	// Finally invoke dialog.
	RunninDialog rd(numThreads, targetType, files, delSrc, isVbr, quality, m_ini, destFolder);
	rd.show(this);
}

void MainDialog::doFileToList(const wstring& file)
{
	using namespace str;

	int iType = -1;
	if (EndsWith(Sens::NO, file, L".mp3"))       iType = 0;
	else if (EndsWith(Sens::NO, file, L".flac")) iType = 1;
	else if (EndsWith(Sens::NO, file, L".wav"))  iType = 2; // what type of audio file is this?

	if (iType == -1) {
		return; // bypass file if unaccepted format
	}
	if (!m_lstFiles.items.exists(file)) {
		m_lstFiles.items.add(file, iType); // add only if not present yet
	}
}

void MainDialog::doUpdateCounter(int newCount)
{
	// Update counter on Run button.
	wstring caption;
	
	if (newCount) caption = str::Sprintf(L"&Run (%d)", newCount);
	else caption = L"&Run";

	this->getChild(BTN_RUN)
		.setText(caption)
		.setEnable(newCount > 0); // Run button enabled if at least 1 file
}