
#pragma once
#include "../wolf/wolf.h"
#include "../res/resource.h"

class MainDialog final : public wolf::wnd::DialogMain {
public:
	MainDialog() : DialogMain(DLG_MAIN, ICO_MAIN) { }
private:
	void events() override;
	INT_PTR _doUpdateCounter(int newCount);
	void    _doFileToList(const std::wstring& file);

	wolf::file::Ini      _ini;
	wolf::ctrl::Resizer  _resizer;
	wolf::ctrl::ListView _lstFiles;
	wolf::ctrl::Combo    _cmbCbr, _cmbVbr, _cmbFlac, _cmbNumThreads;
	wolf::ctrl::Radio    _radMp3, _radMp3Cbr, _radMp3Vbr, _radFlac, _radWav;
	wolf::ctrl::CheckBox _chkDelSrc;
};