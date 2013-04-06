
#include <process.h>
#include "Thread.h"

bool Thread::_run(DWORD timeout, bool halt)
{
	HANDLE handle = (HANDLE)_beginthreadex(0, 0, _Callback, this, 0, 0); // thread starts run here
	bool ret = false;
	
	if(halt) ret = WaitForSingleObject(handle, timeout) != WAIT_TIMEOUT; // halt until return
	else ret = (timeout == INFINITE) ? true : // no halt
		(WaitForSingleObject(handle, timeout) != WAIT_TIMEOUT); // halt until return
	
	CloseHandle(handle);
	return ret;
}

bool Thread::_RunParallel(Array<Thread*> *pThreadPtrs, int msBetweenThreads, DWORD timeout, bool halt)
{
	// Heap-alloc a structure to be sent to the callback function.
	// It holds a copy of the array of thread pointers, so user array can be safely stack-allocated.
	_ParallelThreadInfo *pti = new _ParallelThreadInfo; // produce
	pti->msBetweenThreads = msBetweenThreads;
	pti->threadPtrs.realloc(pThreadPtrs->size());
	memcpy(&pti->threadPtrs[0], &(*pThreadPtrs)[0], pThreadPtrs->size() * sizeof(Thread*));

	// Launch a thread which will launch the user threads.
	HANDLE handle = (HANDLE)_beginthreadex(0, 0, _CallbackParallel, pti, 0, 0);
	bool ret = false;

	if(halt) ret = WaitForSingleObject(handle, timeout) != WAIT_TIMEOUT; // halt until return (synchronous)
	else ret = (timeout == INFINITE) ? true : // no halt
		(WaitForSingleObject(handle, timeout) != WAIT_TIMEOUT); // halt until return

	CloseHandle(handle);
	return ret;
}

unsigned int __stdcall Thread::_Callback(void *pArguments)
{
	{
		Thread *pThreadObj = (Thread*)pArguments; // pointer to us; that's why it should be heap-allocated
		pThreadObj->onRun(); // will halt until return
		delete pThreadObj; // delete thread object (consume); that's why it should never be deleted by user!
	}
	_endthreadex(0); // http://www.codeproject.com/Articles/7732/A-class-to-synchronise-thread-completions/
	return 0;
}

unsigned int __stdcall Thread::_CallbackParallel(void *pArguments)
{
	// This callback is used when running multiple parallel threads.
	// It's an intermediary thread which runs the user threads sychronously.
	// If they had to be asynchronous, _RunParallel() simply won't wait for it.
	{
		_ParallelThreadInfo *pti = (_ParallelThreadInfo*)pArguments;
		Array<HANDLE> handles(pti->threadPtrs.size()); // handles of each user thread to be run
		
		for(int i = 0; i < handles.size(); ++i) {
			if(i > 0 && pti->msBetweenThreads) Sleep(pti->msBetweenThreads); // halt between threads, if demanded
			handles[i] = (HANDLE)_beginthreadex(0, 0,
				_Callback, pti->threadPtrs[i], 0, 0); // user threads start running here
		}
		WaitForMultipleObjects(handles.size(), &handles[0], TRUE, INFINITE); // will halt until all return (synchronous)
		for(int i = 0; i < handles.size(); ++i)
			CloseHandle(handles[i]);
		
		delete pti; // consume; created by _RunParallel()
	}
	_endthreadex(0); // http://www.codeproject.com/Articles/7732/A-class-to-synchronise-thread-completions/
	return 0;
}