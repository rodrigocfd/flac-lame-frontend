#pragma once
#include <mutex>
#include <optional>
#include "../windlg/lib.hpp"
#include <ShObjIdl.h>
#include "../res/resource.h"

class DlgRun final : public wd::BaseDialog {
public:
	enum class Target { mp3, flac, wav };

	struct Opts final {
		const std::wstring lamePath, flacPath;
		const std::vector<std::wstring> files;
		const std::optional<std::wstring> destFolder;
		const Target target;
		const bool delSrc;
		const bool isVbr;
		const std::wstring quality;
		const size_t numThreads;
	};

	explicit DlgRun(Opts &&opts) :
		BaseDialog{DLG_RUN},
		_opts{std::move(opts)}, _hOwner{nullptr}, _taskbar{}, _timer{}, _numFinished{0}, _errors{} { }

private:
	bool on_init_dialog() override;
	void on_close() override;
	void run_tool(size_t idxFile);
	void report_progress();

	wd::Static lblStatus{this, LBL_STATUS};
	wd::ProgressBar proStatus{this, PRO_STATUS};

	Opts _opts;
	HWND _hOwner;
	wd::ComPtr<ITaskbarList3> _taskbar;
	wd::StopWatch _timer;
	size_t _numFinished;
	std::vector<wd::WinErr> _errors;
};
