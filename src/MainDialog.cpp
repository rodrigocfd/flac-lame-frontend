
#include "MainDialog.h"
#include "RunninDialog.h"
#include "Convert.h"

RUN(MainDialog);

INT_PTR MainDialog::msgHandler(UINT msg, WPARAM wp, LPARAM lp)
{
	switch(msg)
	{
	case WM_INITDIALOG: this->onInitDialog(); break;
	case WM_DROPFILES:  this->onDropFiles(wp); return TRUE;

	case WM_COMMAND:
		switch(LOWORD(wp))
		{
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
		switch(((NMHDR*)lp)->idFrom)
		{
		case LST_FILES:
			switch(((NMHDR*)lp)->code)
			{
			case LVN_INSERTITEM:     this->doUpdateCounter(m_lstFiles.items.count()); return TRUE; // new item inserted
			case LVN_DELETEITEM:     this->doUpdateCounter(m_lstFiles.items.count() - 1); return TRUE; // item about to be deleted
			case LVN_DELETEALLITEMS: this->doUpdateCounter(0); return TRUE; // all items about to be deleted
			case LVN_KEYDOWN:
				if(((NMLVKEYDOWN*)lp)->wVKey == VK_DELETE) // DEL
					m_lstFiles.items.removeSelected();
				return TRUE;
			}
			break;
		}
		break;
	}
	return DialogApp::msgHandler(msg, wp, lp); // forward to parent
}

void MainDialog::onEsc()
{
	if(!m_lstFiles.items.count() || this->getChild(BTN_RUN).isEnabled())
		this->sendMessage(WM_CLOSE, 0, 0); // close on ESC only if not processing
}

void MainDialog::onInitDialog()
{
	// Validate and load INI file.
	m_ini.setPath( System::GetExePath().append(L"\\FlacLameFE.ini") );
	
	String err;
	if(!m_ini.load(&err)) {
		this->messageBox(L"Fail", err, MB_ICONERROR);
		this->sendMessage(WM_CLOSE, 0, 0); // halt program
		return;
	}

	// Validate tools.
	if(!Convert::PathsAreValid(m_ini, &err)) {
		this->messageBox(L"Fail", err, MB_ICONERROR);
		this->sendMessage(WM_CLOSE, 0, 0); // halt program
		return;
	}

	// Layout control when resizing.
	m_resizer.create(18)
		.add({ LST_FILES }, this->hWnd(), Resizer::Do::RESIZE, Resizer::Do::RESIZE)
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
	switch(si.dwNumberOfProcessors) {
		case 1: m_cmbNumThreads.itemSetSelected(0); break;
		case 2: m_cmbNumThreads.itemSetSelected(1); break;
		case 4: m_cmbNumThreads.itemSetSelected(2); break;
		case 8: m_cmbNumThreads.itemSetSelected(3); break;
		default: m_cmbNumThreads.itemSetSelected(0);
	}

	// Initializing radio buttons.
	m_radMp3    = this->getChild(RAD_MP3); m_radMp3.setCheck(true, Radio::EmulateClick::EMULATE);
	m_radMp3Cbr = this->getChild(RAD_CBR);
	m_radMp3Vbr = this->getChild(RAD_VBR); m_radMp3Vbr.setCheck(true, Radio::EmulateClick::EMULATE);
	m_radFlac   = this->getChild(RAD_FLAC);
	m_radWav    = this->getChild(RAD_WAV);

	m_chkDelSrc = this->getChild(CHK_DELSRC);
}

void MainDialog::onDropFiles(WPARAM wp)
{
	Array<String> dropFiles = this->getDroppedFiles((HDROP)wp);

	for(int i = 0; i < dropFiles.size(); ++i) {
		if(File::IsDir( dropFiles[i].str() )) { // if a directory, add all files inside of it
			wchar_t subfilebuf[MAX_PATH];
	
			File::Listing findMp3(dropFiles[i], L"*.mp3");
			while(findMp3.next(subfilebuf))
				this->doFileToList(subfilebuf);

			File::Listing findFlac(dropFiles[i], L"*.flac");
			while(findFlac.next(subfilebuf))
				this->doFileToList(subfilebuf);

			File::Listing findWav(dropFiles[i], L"*.wav");
			while(findWav.next(subfilebuf))
				this->doFileToList(subfilebuf);
		} else {
			this->doFileToList(dropFiles[i]); // add single file
		}
	}
	this->doUpdateCounter( m_lstFiles.items.count() );
}

void MainDialog::onChooseDest()
{
	String folder;
	if(this->getFolderChoose(folder))
		this->getChild(TXT_DEST).setText(folder.str()).setFocus();
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
	String destFolder = this->getChild(TXT_DEST).getText();
	
	if(!destFolder.isEmpty()) {
		if(!File::Exists(destFolder)) {
			int q = this->messageBox(L"Create directory",
				String::Fmt(L"The following directory:\n%s\ndoes not exist. Create it?", destFolder.str()),
				MB_ICONQUESTION | MB_YESNO);
			if(q == IDYES) {
				if(!File::CreateDir(destFolder)) {
					this->messageBox(L"Fail",
						String::Fmt(L"The directory failed to be created:\n%s", destFolder.str()), MB_ICONERROR);
					return; // halt
				}
			} else { // user didn't want to create the new dir
				return; // halt
			}
		} else if(!File::IsDir(destFolder)) {
			this->messageBox(L"Fail",
				String::Fmt(L"The following path is not a directory:\n%s", destFolder.str()), MB_ICONERROR);
			return; // halt
		}
	}

	// Check the existence of each file added to list.
	Array<String> files = m_lstFiles.items.getAll().transform<String>(
		[](int i, const ListView::Item& elem)->String { return elem.getText(0); }
	);
	for(int i = 0; i < files.size(); ++i) { // each filepath
		if(!File::Exists(files[i])) {
			this->messageBox(L"Fail",
				String::Fmt(L"Process aborted, file does not exist:\n%s", files[i].str()), MB_ICONERROR);
			return; // halt
		}
	}

	// Retrieve settings.
	bool delSrc = m_chkDelSrc.isChecked();
	bool isVbr = m_radMp3Vbr.isChecked();
	int numThreads = m_cmbNumThreads.itemGetSelectedText().toInt();
	
	String quality;
	if(m_radMp3.isChecked()) {
		Combo& cmbQuality = (isVbr ? m_cmbVbr : m_cmbCbr);
		quality = cmbQuality.itemGetSelectedText();
		quality[quality.findCS(L' ')] = L'\0'; // first characters of chosen option are the quality setting itself
	} else if(m_radFlac.isChecked()) {
		quality = m_cmbFlac.itemGetSelectedText(); // text is quality setting itself
	}

	// Which format are we converting to?
	RunninDialog::Target targetType = RunninDialog::Target::NONE;
	
	if(m_radMp3.isChecked())       targetType = RunninDialog::Target::MP3;
	else if(m_radFlac.isChecked()) targetType = RunninDialog::Target::FLAC;
	else if(m_radWav.isChecked())  targetType = RunninDialog::Target::WAV;

	// Finally invoke dialog.
	RunninDialog rd(numThreads, targetType, files, delSrc, isVbr, quality, m_ini, destFolder);
	rd.show(this);
}

void MainDialog::doFileToList(const String& file)
{
	int iType = -1;
	if(file.endsWithCI(L".mp3"))       iType = 0;
	else if(file.endsWithCI(L".flac")) iType = 1;
	else if(file.endsWithCI(L".wav"))  iType = 2; // what type of audio file is this?

	if(iType == -1)
		return; // bypass file if unaccepted format

	if(!m_lstFiles.items.exists(file))
		m_lstFiles.items.add(file, iType); // add only if not present yet
}

void MainDialog::doUpdateCounter(int newCount)
{
	// Update counter on Run button.
	String caption;
	
	if(newCount) caption = String::Fmt(L"&Run (%d)", newCount);
	else caption = L"&Run";

	this->getChild(BTN_RUN)
		.setText(caption)
		.setEnable(newCount > 0); // Run button enabled if at least 1 file
}