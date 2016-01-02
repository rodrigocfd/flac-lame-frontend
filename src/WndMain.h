
#pragma once
#include "../wolf/wolf.h"

class WndMain final : public wolf::WindowMain {
private:
	wolf::FileIni         _ini;
	wolf::TaskBarProgress _taskBar;
	wolf::Resizer         _resizer;
	wolf::ListView        _lstFiles;
	wolf::ComboBox        _cmbCbr, _cmbVbr, _cmbFlac, _cmbNumThreads;
	wolf::RadioButton     _radMp3, _radMp3Cbr, _radMp3Vbr, _radFlac, _radWav;
	wolf::CheckBox        _chkDelSrc;
public:
	WndMain();
private:
	bool    _destFolderIsOk();
	bool    _filesExist();
	LRESULT _doUpdateCounter(int newCount);
	void    _doFileToList(const std::wstring& file);
};