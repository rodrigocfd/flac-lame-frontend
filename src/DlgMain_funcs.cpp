#define NOMINMAX // https://stackoverflow.com/a/5004874/6923555
#include "DlgMain.h"
#include "convert.h"
#include "../res/resource.h"

void DlgMain::_setNumberOfThreads()
{
	lib::ComboBox cmbThreads{this, CMB_NUMTHREADS};

	SYSTEM_INFO si{};
	GetSystemInfo(&si);

	switch (si.dwNumberOfProcessors) {
		case  2: cmbThreads.select(1); break;
		case  4: cmbThreads.select(2); break;
		case  6: cmbThreads.select(3); break;
		case  8: cmbThreads.select(4); break;
		case 12: cmbThreads.select(5); break;
		default: cmbThreads.select(0);
	}
}

void DlgMain::_loadIniSettings()
{
	std::wstring iniPath = convert::iniPath();
	if (!lib::path::exists(iniPath)) {
		dlg.msgBox(L"No INI file", L"", L"INI file not found at:\n" + iniPath, TDCBF_OK_BUTTON, TD_ERROR_ICON);
		return;
	}

	UINT idxTarget = lib::ini::readInt(iniPath, L"UiSettings", L"target");
	UINT idxMp3Enc = lib::ini::readInt(iniPath, L"UiSettings", L"mp3enc");
	UINT idxCbr = lib::ini::readInt(iniPath, L"UiSettings", L"cbr");
	UINT idxVbr = lib::ini::readInt(iniPath, L"UiSettings", L"vbr");
	UINT idxFlacLvl = lib::ini::readInt(iniPath, L"UiSettings", L"flaclvl");
	UINT bDelOrig = lib::ini::readInt(iniPath, L"UiSettings", L"delorig");

	lib::ComboBox{this, CMB_CBR}.select(idxCbr);
	lib::ComboBox{this, CMB_VBR}.select(idxVbr);
	lib::ComboBox{this, CMB_FLAC}.select(idxFlacLvl);

	switch (idxMp3Enc) {
		case 0: lib::CheckRadio{this, RAD_CBR}.checkAndTrigger(); break;
		case 1: lib::CheckRadio{this, RAD_VBR}.checkAndTrigger();
	}

	switch (idxTarget) {
		case 0: lib::CheckRadio{this, RAD_MP3}.checkAndTrigger(); break;
		case 1: lib::CheckRadio{this, RAD_FLAC}.checkAndTrigger(); break;
		case 2: lib::CheckRadio{this, RAD_WAV}.checkAndTrigger();
	}

	if (bDelOrig) lib::CheckRadio{this, CHK_DELSRC}.checkAndTrigger();
}

void DlgMain::_saveIniSettings()
{
	std::wstring iniPath = convert::iniPath();

	UINT idxTarget = 0;
	if (lib::CheckRadio{this, RAD_FLAC}.isChecked()) idxTarget = 1;
		else if (lib::CheckRadio{this, RAD_WAV}.isChecked()) idxTarget = 2;
	lib::ini::writeInt(iniPath, L"UiSettings", L"target", idxTarget);

	UINT idxMp3Enc = 0;
	if (lib::CheckRadio{this, RAD_VBR}.isChecked()) idxMp3Enc = 1;
	lib::ini::writeInt(iniPath, L"UiSettings", L"mp3enc", idxMp3Enc);

	lib::ini::writeInt(iniPath, L"UiSettings", L"cbr", lib::ComboBox{this, CMB_CBR}.selectedIndex().value());
	lib::ini::writeInt(iniPath, L"UiSettings", L"vbr", lib::ComboBox{this, CMB_VBR}.selectedIndex().value());
	lib::ini::writeInt(iniPath, L"UiSettings", L"flaclvl", lib::ComboBox{this, CMB_FLAC}.selectedIndex().value());
	lib::ini::writeInt(iniPath, L"UiSettings", L"delorig",
		lib::CheckRadio{this, CHK_DELSRC}.isChecked() ? 1 : 0);
}

void DlgMain::_addFileToList(std::wstring_view file)
{
	int ico = -1;
	if (lib::path::hasExtension(file, {L"mp3"})) ico = 0;
	else if (lib::path::hasExtension(file, {L"flac"})) ico = 1;
	else if (lib::path::hasExtension(file, {L"wav"})) ico = 2;

	lib::ListView lv{this, LST_FILES};
	if (!lv.items.find(file).has_value()) { // add only if not present yet
		lib::File f{file, lib::File::Access::ExistingReadOnly};
		size_t fsz = f.size();
		f.close();

		auto szStr = lib::str::fmtBytes(fsz);
		lv.items.add(file, {szStr}, ico)
			.setData(new FileInfo{file, static_cast<int>(fsz)}); // will be deleted in LVN_DELETEITEM
	}
}

void DlgMain::_finishAddingFilesToList()
{
	lib::ListView lv{this, LST_FILES};
	UINT numFiles = lv.items.count();

	if (numFiles) {
		lib::NativeControl{this, BTN_RUN}.setText(
			lib::str::fmt(L"&Run (%d)", numFiles)); // update counter in Run button

		lv.columns[0].setWidthToFill();

		lv.items.sort([this](lib::ListView::Item a, lib::ListView::Item b) -> int {
			auto pNfoA = a.data<FileInfo*>();
			auto pNfoB = b.data<FileInfo*>();
			int cmp = 0;

			if (_sort.col == 0) { // by file path
				cmp = lstrcmpiW(pNfoA->path.c_str(), pNfoB->path.c_str());
			} else if (_sort.col == 1) { // by file size
				cmp = pNfoA->size - pNfoB->size;
			}
			return _sort.asc ? cmp : -cmp;
		});
	} else {
		lib::NativeControl{this, BTN_RUN}.setText(L"&Run"); // no files in the list
	}

	dlg.enable({BTN_RUN}, numFiles > 0);
}

bool DlgMain::_validateDestDir()
{
	lib::NativeControl txtDest{this, TXT_DEST};
	auto destDir = txtDest.text();
	lib::str::trim(destDir);

	if (!destDir.empty() && !lib::path::exists(destDir)) {
		std::wstring msg = L"Destination directory doest not exist:\n" + destDir;
		dlg.msgBox(L"Invalid directory", L"", msg, TDCBF_OK_BUTTON, TD_ERROR_ICON);
		txtDest.focus();
		return false;
	}
	return true;
}

DlgRunnin::Opts DlgMain::_buildOpts()
{
	std::wstring destFolder = lib::NativeControl{this, TXT_DEST}.text();
	lib::str::trim(destFolder);

	std::vector<std::wstring> files = lib::ListView{this, LST_FILES}.columns[0].itemTexts();
	size_t maxThreads = std::stoul(lib::ComboBox{this, CMB_NUMTHREADS}.text());

	DlgRunnin::Opts opts{
		.files = files,
		.destFolder = destFolder.empty() ? std::nullopt : std::optional{destFolder},
		.delSrc = lib::CheckRadio{this, CHK_DELSRC}.isChecked(),
		.isVbr = lib::CheckRadio{this, RAD_VBR}.isChecked(),
		.numThreads = static_cast<BYTE>(std::min(maxThreads, files.size())),
	};

	lib::CheckRadio radMp3{this, RAD_MP3};
	lib::CheckRadio radFlac{this, RAD_FLAC};

	if (radMp3.isChecked()) {
		opts.target = DlgRunnin::Target::Mp3;
		opts.quality = lib::ComboBox{this, static_cast<WORD>(opts.isVbr ? CMB_VBR : CMB_CBR)}.text();
		opts.quality.resize(opts.quality.find_first_of(L' ')); // first characters of chosen option are the quality setting itself
	} else if (radFlac.isChecked()) {
		opts.target = DlgRunnin::Target::Flac;
		opts.quality = lib::ComboBox{this, CMB_FLAC}.text(); // text is quality setting itself
	} else {
		opts.target = DlgRunnin::Target::Wav;
	}

	return opts;
}
