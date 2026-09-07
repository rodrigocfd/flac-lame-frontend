#pragma once
#include "../windlg/lib.hpp"
#include "../res/resource.h"
#include "dlg_run.h"

class DlgMain final : public wd::BaseDialog {
public:
	DlgMain() : BaseDialog{DLG_MAIN, ICO_RONBURGUNDY} { }

private:
	bool on_init_dialog() override;
	bool on_get_min_max_info(MINMAXINFO &mmi) override;
	void on_size(WORD req, SIZE sz) override;
	void on_init_menu_popup(HMENU hMenu) override;
	bool on_command(WORD id, WORD code) override;
	bool on_notify(NMHDR &nm) override;
	void on_destroy() override;

	bool on_drag_files(const std::vector<std::wstring> &files) const;
	void on_drop_files(const std::vector<std::wstring> &files) const;

	void set_initial_number_of_threads() const;
	std::optional<wd::IniFile> try_load_ini() const;
	bool load_ini_settings() const;
	void save_ini_settings() const;
	void menu_open_files() const;
	void menu_rem_selected() const;
	void lst_delete_item(const NMLISTVIEW &nmlv) const;
	void lst_header_click(const NMHEADERW &nmh);
	void btn_dest();
	void rad_click() const;
	void btn_run();
	void add_file_to_list(wd::StrView file) const;
	void update_file_count() const;
	void actual_sort_list_files() const;
	std::optional<DlgRun::Opts> build_run_opts() const;

	wd::ListView lstFiles{this, LST_FILES};
	wd::Edit txtDest{this, TXT_DEST};
	wd::Button btnDest{this, BTN_DEST};

	wd::RadioButton radMp3{this, RAD_MP3};
	wd::RadioButton radFlac{this, RAD_FLAC};
	wd::RadioButton radWav{this, RAD_WAV};

	wd::RadioButton radCbr{this, RAD_CBR};
	wd::RadioButton radVbr{this, RAD_VBR};

	wd::ComboBox cmbCbr{this, CMB_CBR};
	wd::ComboBox cmbVbr{this, CMB_VBR};
	wd::ComboBox cmbFlac{this, CMB_FLAC};

	wd::CheckBox chkDelSrc{this, CHK_DELSRC};
	wd::ComboBox cmbNumThreads{this, CMB_NUMTHREADS};
	wd::Button btnRun{this, BTN_RUN};

	wd::Layout layout{this, 17};
	wd::MenuResource mnuFiles{};
	wd::DropTarget lstFilesDropTarget{};

	POINT _szWndOrig{}; // In screen coords, will feed MINMAXINFO.
	wd::ImageList _imgList{}; // LST_FILES icons
	struct {
		int col;
		wd::ListView::Sort asc;
	} _lstFilesSort{0, wd::ListView::Sort::asc_i}; // State of LST_FILES sorting.

	struct ItemInfo final { // Data to each LST_FILES item.
		size_t size;
	};
};
