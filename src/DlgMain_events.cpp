#include "DlgMain.h"
#include "../res/resource.h"

int APIENTRY wWinMain(_In_ HINSTANCE hInst, _In_opt_ HINSTANCE, _In_ LPWSTR, _In_ int cmdShow)
{
	DlgMain d;
	return lib::runMain(d, hInst, DLG_MAIN, cmdShow, ICO_RONBURGUNDY, ACC_MAIN);
}

INT_PTR DlgMain::dlgProc(UINT uMsg, WPARAM wp, LPARAM lp)
{
	_layout.autoArrange(this, uMsg, wp, lp);
	lib::ListView::ProcessMessages(this, LST_FILES, uMsg, wp, lp, MEN_MAIN);

	switch (uMsg) {
		case WM_INITDIALOG:    return onInitDialog();
		case WM_GETMINMAXINFO: return onGetMinMaxInfo(lp);
		case WM_SIZE:          return onSize(wp, lp);
		case WM_INITMENUPOPUP: return onInitMenuPopup(wp);
		case WM_DROPFILES:     return onDropFiles(wp);
		case WM_COMMAND:
			switch (LOWORD(wp)) {
				case MNU_OPENFILES:   return onMnuOpenFiles();
				case MNU_REMSELECTED: return onMnuRemSelected();
				case MNU_ABOUT:       return onMnuAbout();
				case BTN_DEST:        return onBtnDest();
				case RAD_MP3:
				case RAD_FLAC:
				case RAD_WAV:
				case RAD_CBR:
				case RAD_VBR:         return onRadioClick();
				case BTN_RUN:         return onBtnRun();
				default:              return FALSE;
			}
		case WM_NOTIFY:
			switch (reinterpret_cast<NMHDR*>(lp)->idFrom) {
				case LST_FILES:
					switch (reinterpret_cast<NMHDR*>(lp)->code) {
						case LVN_KEYDOWN:
							switch (reinterpret_cast<NMLVKEYDOWN*>(lp)->wVKey) {
								case VK_DELETE: return onMnuRemSelected();
								default:        return FALSE;
							}
						case LVN_DELETEITEM: return onListDeleteItem(lp);
						case HDN_ITEMCLICK:  return onListHeaderClick(lp);
						default:             return FALSE;
					}
				default: return FALSE;
			}
		case WM_CLOSE:     return onClose();
		case WM_NCDESTROY: PostQuitMessage(0); return TRUE;
		default:           return FALSE;
	}
}

INT_PTR DlgMain::onInitDialog()
{
	_layout.add(lib::Layout::Act::Resize, lib::Layout::Act::Resize, {LST_FILES})
		.add(lib::Layout::Act::None, lib::Layout::Act::Repos, {LBL_DEST, FRA_CONV,
			RAD_MP3, RAD_CBR, CMB_CBR, RAD_VBR, CMB_VBR,
			RAD_FLAC, LBL_LEVEL, CMB_FLAC, RAD_WAV,
			CHK_DELSRC, LBL_NUMTHREADS, CMB_NUMTHREADS})
		.add(lib::Layout::Act::Resize, lib::Layout::Act::Repos, {TXT_DEST})
		.add(lib::Layout::Act::Repos, lib::Layout::Act::Repos, {BTN_DEST, BTN_RUN});

	_imgLst.create({16, 16});
	_imgLst.addShell({L"mp3", L"flac", L"wav"});

	lib::ListView lv{this, LST_FILES};
	lv.setImageList(_imgLst)
		.setFullRowSelect();
	lv.columns.add({
		{L"File", lib::dpi::x(100)},
		{L"Size", lib::dpi::x(70)},
	});
	lv.columns[0].setWidthToFill()
		.setSortArrow(HDF_SORTUP);
	lv.columns[1].setJustification(HDF_CENTER);

	lib::ComboBox{this, CMB_CBR}.add({
		L"32 kbps", L"40 kbps", L"48 kbps", L"56 kbps",
		L"64 kbps", L"80 kbps", L"96 kbps", L"112 kbps",
		L"128 kbps; default",
		L"160 kbps", L"192 kbps", L"224 kbps", L"256 kbps", L"320 kbps",
	});
	lib::ComboBox{this, CMB_VBR}.add({
		L"0 (~245 kbps)", L"1 (~225 kbps)", L"2 (~190 kbps)", L"3 (~175 kbps)",
		L"4 (~165 kbps); default",
		L"5 (~130 kbps)", L"6 (~115 kbps)", L"7 (~100 kbps)", L"8 (~85 kbps)", L"9 (~65 kbps)",
	});
	lib::ComboBox{this, CMB_FLAC}.add({L"1", L"2", L"3", L"4", L"5", L"6", L"7", L"8"});
	lib::ComboBox{this, CMB_NUMTHREADS}.add({L"1", L"2", L"4", L"6", L"8", L"12"});
	_setNumberOfThreads();

	RECT rc{};
	GetWindowRect(hWnd(), &rc);
	_minSize = {.cx = rc.right - rc.left, .cy = rc.bottom - rc.top};

	_loadIniSettings();
	return TRUE;
}

INT_PTR DlgMain::onGetMinMaxInfo(LPARAM lp)
{
	auto pMmi = reinterpret_cast<MINMAXINFO*>(lp);
	pMmi->ptMinTrackSize = {.x = _minSize.cx, .y = _minSize.cy};
	return TRUE;
}

INT_PTR DlgMain::onSize(WPARAM wp, LPARAM lp)
{
	if (wp != SIZE_MINIMIZED)
		lib::ListView{this, LST_FILES}.columns[0].setWidthToFill();
	return TRUE;
}

INT_PTR DlgMain::onInitMenuPopup(WPARAM wp)
{
	lib::Menu popupMenu{reinterpret_cast<HMENU>(wp)};
	if (popupMenu.idByPos(0) == MNU_OPENFILES) {
		popupMenu.enableItemsByCmd({MNU_REMSELECTED},
			lib::ListView{this, LST_FILES}.items.countSelected() > 0);
	}
	return TRUE;
}

INT_PTR DlgMain::onDropFiles(WPARAM wp)
{
	for (const auto& file : dlg.droppedFiles(reinterpret_cast<HDROP>(wp))) {
		if (lib::path::isDir(file)) { // if a directory, add all files inside of it
			for (const auto& subFile : lib::path::dirList(file + L"\\*.mp3"))  _addFileToList(subFile);
			for (const auto& subFile : lib::path::dirList(file + L"\\*.flac")) _addFileToList(subFile);
			for (const auto& subFile : lib::path::dirList(file + L"\\*.wav"))  _addFileToList(subFile);
		} else {
			_addFileToList(file); // add single file
		}
	}
	_finishAddingFilesToList();
	return TRUE;
}

INT_PTR DlgMain::onMnuOpenFiles()
{
	std::optional<std::vector<std::wstring>> files = dlg.showOpenFiles({
		{L"MP3, FLAC and WAV files", L"*.mp3;*.flac;*.wav"},
		{L"MP3 files", L"*.mp3"},
		{L"FLAC files", L"*.flac"},
		{L"WAV files", L"*.wav"},
		{L"All files", L"*.*"},
	});
	if (files.has_value()) {
		for (const auto& file : files.value())
			_addFileToList(file);
		_finishAddingFilesToList();
	}
	return TRUE;
}

INT_PTR DlgMain::onMnuRemSelected()
{
	lib::ListView{this, LST_FILES}.items.removeSelected();
	_finishAddingFilesToList();
	return TRUE;
}

INT_PTR DlgMain::onMnuAbout()
{
	lib::VersionInfo vi;
	std::wstring_view productName = vi.strInfo(vi.langsCps()[0], L"ProductName");
	std::array<WORD, 4> ver = vi.verNum();
	auto body = lib::str::fmt(L"Version %u.%u.%u.\nWritten in C++20.", ver[0], ver[1], ver[2]);

	dlg.msgBox(L"About", productName, body, TDCBF_OK_BUTTON, TD_INFORMATION_ICON);
	return TRUE;
}

INT_PTR DlgMain::onBtnDest()
{
	std::optional<std::wstring> fo = dlg.showOpenFolder();
	if (fo.has_value())
		lib::NativeControl{this, TXT_DEST}.setText(fo.value());
	return TRUE;
}

INT_PTR DlgMain::onListDeleteItem(LPARAM lp)
{
	auto pNmlv = reinterpret_cast<NMLISTVIEW*>(lp);
	auto pNfo = lib::ListView{this, LST_FILES}.items[pNmlv->iItem].data<FileInfo*>();
	delete pNfo;
	return TRUE;
}

INT_PTR DlgMain::onListHeaderClick(LPARAM lp)
{
	auto pNmh = reinterpret_cast<NMHEADERW*>(lp);
	lib::ListView lv{this, LST_FILES};
	int arrowFlag = lv.columns[pNmh->iItem].sortArrow();
	bool willSortAsc = !(arrowFlag & HDF_SORTUP);

	lv.columns[pNmh->iItem].setSortArrow(willSortAsc ? HDF_SORTUP : HDF_SORTDOWN); // draw arrow
	_sort = {.col = pNmh->iItem, .asc = willSortAsc}; // update state
	_finishAddingFilesToList(); // sort the files
	return TRUE;
}

INT_PTR DlgMain::onRadioClick()
{
	bool isMp3 = lib::CheckRadio{this, RAD_MP3}.isChecked();
	dlg.enable({RAD_CBR, CMB_CBR, RAD_VBR, CMB_VBR}, isMp3);
	dlg.enable({LBL_LEVEL, CMB_FLAC}, lib::CheckRadio{this, RAD_FLAC}.isChecked());
	dlg.enable({CMB_CBR}, isMp3 && lib::CheckRadio{this, RAD_CBR}.isChecked());
	dlg.enable({CMB_VBR}, isMp3 && lib::CheckRadio{this, RAD_VBR}.isChecked());
	return TRUE;
}

INT_PTR DlgMain::onBtnRun()
{
	if (_validateDestDir()) {
		auto opts = _buildOpts();
		DlgRunnin d{std::move(opts)};
		d.showModal(this, DLG_RUNNIN);

		FLASHWINFO fwi = {
			.cbSize = sizeof(FLASHWINFO),
			.hwnd = hWnd(),
			.dwFlags = FLASHW_TRAY | FLASHW_TIMERNOFG,
		};
		FlashWindowEx(&fwi);
	}
	return TRUE;
}

INT_PTR DlgMain::onClose()
{
	_saveIniSettings();
	DestroyWindow(hWnd());
	return TRUE;
}
