#pragma once
#include <windlg/lib.h>
#include "DlgRunnin.h"

class DlgMain final : public lib::DialogMain {
private:
	struct FileInfo final {
		std::wstring path;
		int size = 0;
		FileInfo(std::wstring_view p, int s) : path{p}, size{s} { }
	};

	SIZE _minSize{};
	lib::Layout _layout;
	lib::ImgList _imgLst;
	struct { int col; bool asc; } _sort = {.col = 0, .asc = true};

public:
	virtual ~DlgMain() { }

	constexpr DlgMain() = default;
	DlgMain(const DialogMain&) = delete;
	DlgMain(DialogMain&&) = delete;
	DlgMain& operator=(const DlgMain&) = delete;
	DlgMain& operator=(DlgMain&&) = delete;

private:
	INT_PTR dlgProc(UINT uMsg, WPARAM wp, LPARAM lp) override;
	INT_PTR onInitDialog();
	INT_PTR onGetMinMaxInfo(LPARAM lp);
	INT_PTR onSize(WPARAM wp, LPARAM lp);
	INT_PTR onInitMenuPopup(WPARAM wp);
	INT_PTR onDropFiles(WPARAM wp);
	INT_PTR onMnuOpenFiles();
	INT_PTR onMnuRemSelected();
	INT_PTR onMnuAbout();
	INT_PTR onBtnDest();
	INT_PTR onListDeleteItem(LPARAM lp);
	INT_PTR onListHeaderClick(LPARAM lp);
	INT_PTR onRadioClick();
	INT_PTR onBtnRun();
	INT_PTR onClose();

	void _loadIniSettings();
	void _saveIniSettings();
	void _addFileToList(std::wstring_view file);
	void _finishAddingFilesToList();
	bool _validateDestDir();
	DlgRunnin::Opts _buildOpts();
};
