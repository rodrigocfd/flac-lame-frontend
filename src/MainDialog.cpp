
#define _CRT_SECURE_NO_DEPRECATE // _itow() VC2005
#define _CRT_SECURE_NO_WARNINGS // _itow() VC2008

#include <direct.h> // _wmkdir()
#include "../toolow/DropFiles.h"
#include "../toolow/File.h"
#include "../toolow/util.h"
#include "MainDialog.h"
#include "RunninDialog.h"
#include "Converter.h"
#include "../res/resource.h"

INT_PTR MainDialog::msgHandler(UINT msg, WPARAM wp, LPARAM lp)
{
	switch(msg)
	{
	case WM_INITDIALOG: on_initDialog(); break;
	case WM_SIZE:       m_resizer.doResize(wp, lp); m_lstFiles.columnFit(0); return TRUE;
	case WM_DROPFILES:  on_dropFiles(wp); return TRUE;

	case WM_COMMAND:
		switch(LOWORD(wp))
		{
		case IDCANCEL: on_esc(); return TRUE; // close on ESC
		case BTN_DEST: on_chooseDest(); return TRUE;
		case RAD_MP3:
		case RAD_FLAC:
		case RAD_WAV:  on_selectFormat(); return TRUE;
		case RAD_CBR:
		case RAD_VBR:  on_selectRate(); return TRUE;
		case BTN_RUN:  on_run(); return TRUE;
		}
		break;

	case WM_NOTIFY:
		switch(((NMHDR*)lp)->idFrom)
		{
		case LST_FILES:
			switch(((NMHDR*)lp)->code)
			{
			case LVN_INSERTITEM:     do_updateCounter(m_lstFiles.items.count()); return TRUE; // new item inserted
			case LVN_DELETEITEM:     do_updateCounter(m_lstFiles.items.count() - 1); return TRUE; // item about to be deleted
			case LVN_DELETEALLITEMS: do_updateCounter(0); return TRUE; // all items about to be deleted
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

void MainDialog::on_esc()
{
	if(!m_lstFiles.items.count() || this->getChild(BTN_RUN).isEnabled())
		this->sendMessage(WM_CLOSE, 0, 0); // close on ESC only if not processing
}

void MainDialog::on_initDialog()
{
	// Validate and load INI file.
	String iniPath;
	GetPathTo::Exe(&iniPath)->append(L"\\FlacLameFE.ini");
	m_ini.setPath(iniPath.str());

	String err;
	if(!m_ini.load(&err)) {
		this->messageBox(L"Fail", err.str(), MB_ICONERROR);
		return;
	}

	// Validate tools.
	if(!Converter::PathsAreValid(&m_ini, &err)) {
		this->messageBox(L"Fail", err.str(), MB_ICONERROR);
		this->sendMessage(WM_CLOSE, 0, 0); // halt program
		return;
	}

	// Layout control when resizing.
	m_resizer.create(16)
		.add(this->hWnd(), LST_FILES, Resizer::RESIZE, Resizer::RESIZE)
		.add(this->hWnd(), TXT_DEST, Resizer::RESIZE, Resizer::REPOS)
		.addById(Resizer::RENONE, Resizer::REPOS, this->hWnd(), 12,
			LBL_DEST, FRA_CONV, RAD_MP3, RAD_FLAC, RAD_WAV, RAD_CBR, RAD_VBR, LBL_LEVEL, CMB_CBR, CMB_VBR, CMB_FLAC, CHK_DELSRC)
		.addById(Resizer::REPOS, Resizer::REPOS, this->hWnd(), 2,
			BTN_DEST, BTN_RUN);

	// Main listview initialization.
	m_lstFiles = this->getChild(LST_FILES);
	m_lstFiles.columnAdd(L"File", 300)
		.columnFit(0)
		.iconPush(L"mp3")
		.iconPush(L"flac")
		.iconPush(L"wav"); // icons of the 3 filetypes we use

	// Initializing comboboxes.
	m_cmbCbr = this->getChild(CMB_CBR);
	const wchar_t *cbrRates[] = {
		L"32 kbps", L"40 kbps", L"48 kbps", L"56 kbps", L"64 kbps", L"80 kbps", L"96 kbps", L"112 kbps",
		L"128 kbps; default", L"160 kbps", L"192 kbps", L"224 kbps", L"256 kbps", L"320 kbps" };
	m_cmbCbr.itemAdd(ARRAYSIZE(cbrRates), cbrRates);
	m_cmbCbr.itemSetSelected(8);

	m_cmbVbr = this->getChild(CMB_VBR);
	const wchar_t *vbrRates[] = {
		L"0 (~245 kbps)", L"1 (~225 kbps)", L"2 (~190 kbps)", L"3 (~175 kbps)", L"4 (~165 kbps); default",
		L"5 (~130 kbps)", L"6 (~115 kbps)", L"7 (~100 kbps)", L"8 (~85 kbps)", L"9 (~65 kbps)" };
	m_cmbVbr.itemAdd(ARRAYSIZE(vbrRates), vbrRates);
	m_cmbVbr.itemSetSelected(4);

	m_cmbFlac = this->getChild(CMB_FLAC);
	for(int i = 1; i <= 8; ++i) {
		wchar_t num[8];
		_itow(i, num, 10);
		m_cmbFlac.itemAdd(num);
	}
	m_cmbFlac.itemSetSelected(7);

	m_cmbNumThreads = this->getChild(CMB_NUMTHREADS);
	const wchar_t *numThreads[] = { L"1", L"2", L"4", L"8" };
	m_cmbNumThreads.itemAdd(ARRAYSIZE(numThreads), numThreads);
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

void MainDialog::on_dropFiles(WPARAM wp)
{
	DropFiles drop((HDROP)wp);

	for(int i = 0; i < drop.count(); ++i)
	{
		wchar_t filebuf[MAX_PATH];
		drop.get(i, filebuf); // retrieve dropped file path

		if(File::IsDir(filebuf)) // if a directory, add all files inside of it
		{
			wchar_t subfilebuf[MAX_PATH];
	
			File::Enum findMp3(filebuf, L"*.mp3");
			while(findMp3.next(subfilebuf))
				do_fileToList(subfilebuf);

			File::Enum findFlac(filebuf, L"*.flac");
			while(findFlac.next(subfilebuf))
				do_fileToList(subfilebuf);

			File::Enum findWav(filebuf, L"*.wav");
			while(findWav.next(subfilebuf))
				do_fileToList(subfilebuf);
		}
		else
			do_fileToList(filebuf); // add single file
	}

	do_updateCounter( m_lstFiles.items.count() );
}

void MainDialog::on_chooseDest()
{
	String folder;
	if(this->getFolderChoose(&folder))
		this->getChild(TXT_DEST).setText(folder.str()).setFocus();
}

void MainDialog::on_selectFormat()
{
	m_radMp3Cbr.setEnable( m_radMp3.isChecked() );
	m_cmbCbr.setEnable( m_radMp3.isChecked() && m_radMp3Cbr.isChecked() );

	m_radMp3Vbr.setEnable( m_radMp3.isChecked() );
	m_cmbVbr.setEnable( m_radMp3.isChecked() && m_radMp3Vbr.isChecked() );

	this->getChild(LBL_LEVEL).setEnable( m_radFlac.isChecked() );
	m_cmbFlac.setEnable( m_radFlac.isChecked() );
}

void MainDialog::on_selectRate()
{
	m_cmbCbr.setEnable( m_radMp3Cbr.isChecked() );
	m_cmbVbr.setEnable( m_radMp3Vbr.isChecked() );
}

void MainDialog::on_run()
{
	// Validate destination folder, if any.
	wchar_t destFolder[MAX_PATH];
	this->getChild(TXT_DEST).getText(destFolder, ARRAYSIZE(destFolder));
	
	if(lstrlen(destFolder)) {
		if(!File::Exists(destFolder)) {
			int q = this->messageBox(L"Create directory",
				fmt(L"The following directory:\n%s\ndoes not exist. Create it?", destFolder)->str(),
				MB_ICONQUESTION | MB_YESNO);
			if(q == IDYES) {
				if(_wmkdir(destFolder) != 0) {
					this->messageBox(L"Fail", fmt(L"The directory failed to be created:\n%s", destFolder)->str(), MB_ICONERROR);
					return; // halt
				}
			} else {
				return; // halt
			}
		} else if(!File::IsDir(destFolder)) {
			this->messageBox(L"Fail", fmt(L"The following path is not a directory:\n%s", destFolder)->str(), MB_ICONERROR);
			return; // halt
		}
	}
	wchar_t *pDestFolder = *destFolder ? destFolder : NULL; // path to output dir we'll use

	// Check the existence of each file added to list.
	for(int i = 0; i < m_lstFiles.items.count(); ++i) {
		wchar_t filebuf[MAX_PATH];
		if( !File::Exists(m_lstFiles.items[i].getText(filebuf, ARRAYSIZE(filebuf))) ) {
			this->messageBox(L"Fail", fmt(L"File does not exist:\n%s", filebuf)->str(), MB_ICONERROR);
			return; // halt
		}
	}
	Array<String> *pFiles = new Array<String>; // will be consumed by running dialog
	m_lstFiles.items.getAllText(pFiles, 0);

	// Retrieve settings.
	bool delSrc = m_chkDelSrc.isChecked();
	bool isVbr = m_radMp3Vbr.isChecked();

	wchar_t chosenNumThread[8];
	m_cmbNumThreads.itemGetText(m_cmbNumThreads.itemGetSelected(), chosenNumThread, ARRAYSIZE(chosenNumThread));
	int numThreads = _wtoi(chosenNumThread);
	
	wchar_t quality[32] = { 0 };
	if(m_radMp3.isChecked()) {
		Combo *pCmbQ = (isVbr ? &m_cmbVbr : &m_cmbCbr);
		int ii = pCmbQ->itemGetSelected();
		pCmbQ->itemGetText(pCmbQ->itemGetSelected(), quality, ARRAYSIZE(quality));
		*wcschr(quality, L' ') = 0; // first characters of chosen option are the quality setting itself
	}
	else if(m_radFlac.isChecked())
		m_cmbFlac.itemGetText(m_cmbFlac.itemGetSelected(), quality, ARRAYSIZE(quality)); // text is quality setting itself

	// Which format are we converting to?
	RunninDialog::Target::Type targetType = RunninDialog::Target::NONE;
	
	if(m_radMp3.isChecked())       targetType = RunninDialog::Target::MP3;
	else if(m_radFlac.isChecked()) targetType = RunninDialog::Target::FLAC;
	else if(m_radWav.isChecked())  targetType = RunninDialog::Target::WAV;

	// Finally invoke dialog.
	RunninDialog rd(numThreads, targetType, pFiles, delSrc, isVbr, quality, &m_ini, pDestFolder);
	rd.show(this);
}

void MainDialog::do_fileToList(const wchar_t *file)
{
	int iType = -1;
	String sFile = file;
	if(sFile.endsWith(L".mp3", String::INSENS))       iType = 0;
	else if(sFile.endsWith(L".flac", String::INSENS)) iType = 1;
	else if(sFile.endsWith(L".wav", String::INSENS))  iType = 2; // what type of audio file is this?

	if(iType == -1)
		return; // bypass file if unaccepted format

	if(!m_lstFiles.items.exists(sFile.str()))
		m_lstFiles.items.add(sFile.str(), iType); // add only if not present yet
}

void MainDialog::do_updateCounter(int newCount)
{
	// Update counter on Run button.
	String caption;
	
	if(newCount) caption.fmt(L"&Run (%d)", newCount);
	else caption = L"&Run";

	this->getChild(BTN_RUN)
		.setText(caption.str())
		.setEnable(newCount > 0); // Run button enabled if at least 1 file
}