#include <mutex>
#include "dlg_run.h"
#include "convert.h"

bool DlgRun::on_init_dialog() {
	_hOwner = GetWindow(hwnd(), GW_OWNER);
	_taskbar.co_create_instance(CLSID_TaskbarList);
	_taskbar->SetProgressState(_hOwner, TBPF_NORMAL);
	_taskbar->SetProgressValue(_hOwner, 0, _opts.files.size());

	lblStatus.set_text(wd::str::fmt(L"Processing first %u file(s)...", _opts.numThreads));
	proStatus.set_range(0, _opts.numThreads);
	_timer.restart();

	wd::run_thread_pool(_opts.files.size(), [this](size_t index) {
		run_tool(index);
		wd::run_ui_thread(this, [this]() {
			report_progress();
		});
	}, _opts.numThreads);

	return true;
}

void DlgRun::on_close() {
	// Don't call EndDialog(), so user can't close it.
}

void DlgRun::run_tool(size_t idxFile) {
	const std::wstring &file = _opts.files[idxFile];
	try {
		switch (_opts.target) {
		case Target::mp3:
			convert::to_mp3(_opts.lamePath, _opts.flacPath, file, _opts.destFolder, _opts.delSrc, _opts.quality, _opts.isVbr);
			break;
		case Target::flac:
			convert::to_flac(_opts.lamePath, _opts.flacPath, file, _opts.destFolder, _opts.delSrc, _opts.quality);
			break;
		case Target::wav:
			convert::to_wav(_opts.lamePath, _opts.flacPath, file, _opts.destFolder, _opts.delSrc);
		}
	} catch (const wd::WinErr &e) {
		static std::mutex mutex{};
		std::lock_guard lock{mutex};
		_errors.emplace_back(e); // store the error and keep going
	}
}

void DlgRun::report_progress() {
	++_numFinished;
	if (_numFinished < _opts.files.size() && _errors.empty()) {
		_taskbar->SetProgressValue(_hOwner, _numFinished, _opts.files.size());
		proStatus.set_pos(_numFinished);
		lblStatus.set_text(wd::str::fmt(L"%u of %u files finished...", _numFinished, _opts.files.size()));
	} else if (_numFinished == _opts.files.size()) {
		if (_errors.empty()) [[likely]] {
			std::wstring ellapsedFmt = _timer.ellapsed_fmt();
			std::wstring msg = wd::str::fmt(L"%u file(s) converted with %u thread(s) in %s.",
				_opts.files.size(), _opts.numThreads, ellapsedFmt);
			wd::sys_dlg::msg_ok(this, L"File(s) finished", msg);
			_taskbar->SetProgressState(_hOwner, TBPF_NOPROGRESS);
			EndDialog(hwnd(), 0);
		} else {
			_taskbar->SetProgressState(_hOwner, TBPF_ERROR);
			proStatus.set_state(wd::ProgressBar::State::error);
			wd::sys_dlg::msg_err(this, L"Conversion failed",
				wd::str::fmt(L"%u error(s), first is:", _errors.size()),
				_errors[0]); // show first error only
			_taskbar->SetProgressState(_hOwner, TBPF_NOPROGRESS);
			proStatus.set_state(wd::ProgressBar::State::normal);
			EndDialog(hwnd(), 1);
		}
	}
}
