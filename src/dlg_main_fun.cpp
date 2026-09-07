#include "dlg_main.h"

bool DlgMain::on_drag_files(const std::vector<std::wstring> &files) const {
	for (auto &&file : files) {
		if (wd::file::is_dir(file)) {
			for (auto &&subFile : wd::DirList{file}) { // go only 1 level deep
				if (wd::StrView{subFile}.ends_with_i({L".mp3", L".flac", L".wav"}))
					return true;
			}
		} else {
			if (wd::StrView{file}.ends_with_i({L".mp3", L".flac", L".wav"}))
				return true;
		}
	}
	return false;
}

void DlgMain::on_drop_files(const std::vector<std::wstring> &files) const {
	for (auto &&file : files) {
		if (wd::file::is_dir(file)) {
			for (auto &&subFile : wd::DirList{file}) { // go only 1 level deep
				if (wd::StrView{subFile}.ends_with_i({L".mp3", L".flac", L".wav"}))
					add_file_to_list(subFile);
			}
		} else {
			if (wd::StrView{file}.ends_with_i({L".mp3", L".flac", L".wav"}))
				add_file_to_list(file);
		}
	}
	update_file_count();
	actual_sort_list_files();
}

void DlgMain::set_initial_number_of_threads() const {
	SYSTEM_INFO si{};
	GetSystemInfo(&si);

	cmbNumThreads.item_add(std::to_wstring(1)); // minimum 1
	for (DWORD i = 2; i <= si.dwNumberOfProcessors; i += 2)
		cmbNumThreads.item_add(std::to_wstring(i));

	cmbNumThreads.item(-1).select();
}

std::optional<wd::IniFile> DlgMain::try_load_ini() const {
	const std::wstring iniPath = wd::file::exe_dir() + L"\\flac-lame-frontend.ini";
	wd::IniFile iniFile{};
	try {
		iniFile.load(iniPath);
	} catch (const wd::WinErr &e) {
		wd::sys_dlg::msg_err(this, L"Load INI failed", L"Failed to load:\n" + iniPath, e);
		return std::nullopt;
	}
	return std::optional{iniFile};
}

bool DlgMain::load_ini_settings() const {
	std::optional<wd::IniFile> iniFile = try_load_ini();
	if (!iniFile.has_value()) [[unlikely]] {
		return false;
	}

	wd::IniFile::Section *pUiSettings = iniFile->get(L"UiSettings");
	if (!pUiSettings) [[unlikely]] {
		wd::sys_dlg::msg_err(this, L"Load INI failed", L"Invalid INI file content.");
		return false;
	}

	UINT idxTarget  = pUiSettings->read_uint(L"target");
	UINT idxMp3Enc  = pUiSettings->read_uint(L"mp3enc");
	UINT idxCbr     = pUiSettings->read_uint(L"cbr");
	UINT idxVbr     = pUiSettings->read_uint(L"vbr");
	UINT idxFlacLvl = pUiSettings->read_uint(L"flaclvl");
	UINT bDelOrig   = pUiSettings->read_uint(L"delorig");

	cmbCbr.item(idxCbr).select();
	cmbVbr.item(idxVbr).select();
	cmbFlac.item(idxFlacLvl).select();

	switch (idxMp3Enc) {
		case 0: radCbr.set_check_and_trigger(true); break;
		case 1: radVbr.set_check_and_trigger(true);
	}
	switch (idxTarget) {
		case 0: radMp3.set_check_and_trigger(true); break;
		case 1: radFlac.set_check_and_trigger(true); break;
		case 2: radWav.set_check_and_trigger(true);
	}

	chkDelSrc.set_check(bDelOrig);
	return true;
}

void DlgMain::save_ini_settings() const {
	std::optional<wd::IniFile> iniFile = try_load_ini();
	if (!iniFile.has_value()) [[unlikely]] {
		return;
	}

	wd::IniFile::Section *pUiSettings = iniFile->get(L"UiSettings");

	UINT idxTarget = 0;
	if (radFlac.is_checked()) idxTarget = 1;
		else if (radWav.is_checked()) idxTarget = 2;
	pUiSettings->write(L"target", idxTarget);

	UINT idxMp3Enc = 0;
	if (radVbr.is_checked()) idxMp3Enc = 1;
	pUiSettings->write(L"mp3enc", idxMp3Enc);

	pUiSettings->write(L"cbr", cmbCbr.item_selected().index());
	pUiSettings->write(L"vbr", cmbVbr.item_selected().index());
	pUiSettings->write(L"flaclvl", cmbFlac.item_selected().index());
	pUiSettings->write(L"delorig", chkDelSrc.is_checked() ? 1 : 0);

	iniFile->save();
}

void DlgMain::menu_open_files() const {
	const std::vector<std::wstring> files = wd::sys_dlg::file_open_multi(this, {
		{L"MP3, FLAC and WAV files", L"*.mp3;*.flac;*.wav"},
		{L"MP3 files", L"*.mp3"},
		{L"FLAC files", L"*.flac"},
		{L"WAV files", L"*.wav"},
		{L"All files", L"*.*"},
	});
	if (!files.empty()) {
		for (auto &&filePath : files)
			add_file_to_list(filePath);
		update_file_count();
		actual_sort_list_files();
	}
}

void DlgMain::menu_rem_selected() const {
	lstFiles.item_del_selected();
	update_file_count();
}

void DlgMain::lst_delete_item(const NMLISTVIEW &nmlv) const {
	ItemInfo *pInfo = lstFiles.item(nmlv.iItem).data<ItemInfo*>();
	delete pInfo;
}

void DlgMain::lst_header_click(const NMHEADERW &nmh) {
	using Lv = wd::ListView;
	if (int newCol = nmh.iItem; newCol == _lstFilesSort.col) {
		_lstFilesSort.asc = (_lstFilesSort.asc == Lv::Sort::asc_i)
			? Lv::Sort::desc_i
			: Lv::Sort::asc_i;
	} else {
		_lstFilesSort.col = newCol;
		_lstFilesSort.asc = Lv::Sort::asc_i;
	}
	actual_sort_list_files();
}

void DlgMain::btn_dest() {
	const std::wstring newFolder = wd::sys_dlg::folder_open(this, nullptr);
	if (!newFolder.empty())
		txtDest.set_text(newFolder);
}

void DlgMain::rad_click() const {
	wd::enable_many(this, radMp3.is_checked(), {RAD_CBR, CMB_CBR, RAD_VBR, CMB_VBR});
	wd::enable_many(this, radFlac.is_checked(), {LBL_LEVEL, CMB_FLAC});
	wd::enable_many(this, radMp3.is_checked() && radCbr.is_checked(), {CMB_CBR});
	wd::enable_many(this, radMp3.is_checked() && radVbr.is_checked(), {CMB_VBR});
}

void DlgMain::btn_run() {
	std::optional<DlgRun::Opts> opts = build_run_opts();
	if (!opts.has_value()) [[unlikely]] {
		return;
	}

	DlgRun dlg{std::move(opts.value())};
	wd::run_modal_dialog(this, dlg);
}

void DlgMain::add_file_to_list(wd::StrView filePath) const {
	size_t szBytes = 0;
	try {
		szBytes = wd::file::size(filePath);
	} catch (const wd::WinErr &e) {
		wd::sys_dlg::msg_err(this, L"Load failed", L"Failed to read size of:\n" + filePath, e);
		return;
	}
	const std::wstring strBytes = wd::str::fmt_bytes(szBytes);

	if (wd::ListView::Item item = lstFiles.item_find(filePath, 0); item.valid()) { // item already present
		item.set_text(1, strBytes);
		item.data<ItemInfo*>()->size = szBytes;
	} else {
		int ico = -1;
		if (filePath.ends_with_i(L".mp3")) ico = 0;
			else if (filePath.ends_with_i(L".flac")) ico = 1;
			else if (filePath.ends_with_i(L".wav")) ico = 2;
			else return; // don't add invalid file; should never happen

		wd::ListView::Item newItem = lstFiles.item_add({filePath, strBytes}, ico);
		const ItemInfo *pNewInfo = new ItemInfo{szBytes};
		newItem.set_data(pNewInfo); // deleted in LVN_DELETEITEM
	}
}

void DlgMain::update_file_count() const {
	if (size_t numFiles = lstFiles.item_count(); numFiles) {
		btnRun.set_text(wd::str::fmt(L"&Run (%u)", numFiles)).enable(true);
		lstFiles.col(0).set_width_to_fill();
	} else {
		btnRun.set_text(L"&Run").enable(false);
	}
}

void DlgMain::actual_sort_list_files() const {
	if (_lstFilesSort.col == 0) {
		lstFiles.sort({_lstFilesSort.col, _lstFilesSort.asc});
	} else {	
		using Lv = wd::ListView;
		bool isAsc = _lstFilesSort.asc == Lv::Sort::asc_i;
		lstFiles.sort([isAsc](Lv::Item a, Lv::Item b) -> int {
			ItemInfo *pInfoA = a.data<ItemInfo*>();
			ItemInfo *pInfoB = b.data<ItemInfo*>();
			return isAsc
				? static_cast<int>(pInfoA->size) - static_cast<int>(pInfoB->size)
				: static_cast<int>(pInfoB->size) - static_cast<int>(pInfoA->size);
		});
		lstFiles.col(0).set_arrow(Lv::Arrow::none);
		lstFiles.col(1).set_arrow(isAsc ? Lv::Arrow::asc : Lv::Arrow::desc);
	}
}

std::optional<DlgRun::Opts> DlgMain::build_run_opts() const {
	std::optional<wd::IniFile> iniFile = try_load_ini();
	if (!iniFile.has_value()) [[unlikely]] {
		return std::nullopt;
	}

	const wd::IniFile::Section *pTools = iniFile->get(L"Tools");
	std::wstring lamePath = pTools->read_str(L"lame");
	std::wstring flacPath = pTools->read_str(L"flac");

	std::vector<std::wstring> files = lstFiles.col(0).texts();

	std::wstring destFolder = txtDest.text();
	wd::str::trim_spaces(destFolder);
	if (!destFolder.empty() && !wd::file::exists(destFolder)) [[unlikely]] {
		wd::sys_dlg::msg_err(this, L"Invalid directory", wd::str::fmt(L"Destination directory doesn't exist:\n%s", destFolder));
		txtDest.focus();
		return std::nullopt;
	}

	DlgRun::Target target = DlgRun::Target::wav;
	const bool delSrc = chkDelSrc.is_checked();
	const bool isVbr = radVbr.is_checked();
	std::wstring quality{};
	if (radMp3.is_checked()) {
		target = DlgRun::Target::mp3;
		quality = isVbr ? cmbVbr.text() : cmbCbr.text();
		quality.resize(quality.find_first_of(L' ')); // first characters of chosen option are the quality setting itself
	} else if (radFlac.is_checked()) {
		target = DlgRun::Target::flac;
		quality = cmbFlac.text(); // text is quality setting itself
	}

	const size_t maxThreads = static_cast<size_t>(std::stoull(cmbNumThreads.text()));
	const size_t numFiles = files.size();

	DlgRun::Opts opts{
		.lamePath = std::move(lamePath),
		.flacPath = std::move(flacPath),
		.files = std::move(files),
		.destFolder = destFolder.empty() ? std::nullopt : std::optional{std::move(destFolder)},
		.target = target,
		.delSrc = delSrc,
		.isVbr = isVbr,
		.quality = std::move(quality),
		.numThreads = std::min(maxThreads, numFiles),
	};
	return std::optional{std::move(opts)};
}
