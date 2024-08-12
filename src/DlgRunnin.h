#pragma once
#include <mutex>
#include <optional>
#include <string>
#include <vector>
#include <Windows.h>
#include <ShObjIdl.h>
#include <windlg/lib.h>

class DlgRunnin final : public lib::DialogModal {
public:
	enum class Target { Mp3, Flac, Wav };

	struct Opts final {
		std::vector<std::wstring> files;
		std::optional<std::wstring> destFolder;
		Target target;
		bool delSrc;
		bool isVbr;
		std::wstring quality;
		BYTE numThreads;
	};

	virtual ~DlgRunnin() { }

	constexpr explicit DlgRunnin(Opts&& opts) : _opts{std::move(opts)} { }
	DlgRunnin(const DlgRunnin&) = delete;
	DlgRunnin(DlgRunnin&&) = delete;
	DlgRunnin& operator=(const DlgRunnin&) = delete;
	DlgRunnin& operator=(DlgRunnin&&) = delete;

private:
	INT_PTR dlgProc(UINT uMsg, WPARAM wp, LPARAM lp) override;
	INT_PTR onInitDialog();

	void _processNextFileDetached();
	bool _launchConvertProcess(UINT idxFile);

	lib::ComPtr<ITaskbarList4> _taskbar;
	Opts _opts;
	UINT _idxNextFile = 0;
	UINT _numFilesDone = 0;
	std::mutex _mutex;
	lib::TimeCount _time = lib::TimeCount::Delayed();
};
