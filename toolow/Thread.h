//
// Automation for threads.
// Part of TOOLOW - Thin Object Oriented Layer Over Win32.
//

#pragma once
#include "Array.h"
#include <Windows.h>

//
// USAGE:
// Inherit from this class and implement onRun().
// When instantiating, *ALWAYS* allocate on heap and *DO NOT* delete it; delete occurs automatically at thread end.
// Sync methods will halt until return; async will return immediately.
//
class Thread {
public:
	virtual ~Thread() { }

	bool runAsync(DWORD timeout=INFINITE) { return _run(timeout, false); }
	bool runSync(DWORD timeout=INFINITE)  { return _run(timeout, true); }
	static bool RunParallelAsync(Array<Thread*> *pThreadPtrs, int msBetweenThreads=0, DWORD timeout=INFINITE) { return _RunParallel(pThreadPtrs, msBetweenThreads, timeout, false); }
	static bool RunParallelSync(Array<Thread*> *pThreadPtrs, int msBetweenThreads=0, DWORD timeout=INFINITE)  { return _RunParallel(pThreadPtrs, msBetweenThreads, timeout, true); }

private:
	virtual void onRun() = 0;

	bool _run(DWORD timeout, bool halt);
	struct _ParallelThreadInfo { Array<Thread*> threadPtrs; int msBetweenThreads; };
	static bool _RunParallel(Array<Thread*> *pThreadPtrs, int msBetweenThreads, DWORD timeout, bool halt);
	static unsigned int __stdcall _Callback(void *pArguments);
	static unsigned int __stdcall _CallbackParallel(void *pArguments);
};