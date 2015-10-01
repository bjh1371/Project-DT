////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file Exception.cpp
/// \author bjh1371
/// \date 2015/06/08
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Exception.h"

#include <DbgHelp.h>
#include <strsafe.h>
#include <tlhelp32.h> 

#include "StackWalker.h"
#include "ThreadRegistry.h"

#pragma comment(lib, "DbgHelp")

#define MAX_BUF_SIZE 1024

namespace
{
	void MakeDump(EXCEPTION_POINTERS* e)
	{
		TCHAR fileName[MAX_BUF_SIZE] = {0};

		SYSTEMTIME t;
		GetLocalTime(&t);

		_stprintf_s(
		 fileName, ARRAYSIZE(fileName),
		 _T("%04d%c%02d%c%02d")
		 _T("%c")
		 _T("%02d%c%02d%c%02d.dmp"),
		 t.wYear, _T('-'), t.wMonth, _T('-'), t.wDay,
		 _T(' '),
		 t.wHour, _T('-'), t.wMinute, _T('-'), t.wSecond, _T('-'),
		 _TRUNCATE
		 );

		HANDLE hFile = CreateFile(fileName, GENERIC_WRITE, FILE_SHARE_READ, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
		if (hFile == INVALID_HANDLE_VALUE)
			return;

		MINIDUMP_EXCEPTION_INFORMATION exceptionInfo;
		exceptionInfo.ThreadId = GetCurrentThreadId();
		exceptionInfo.ExceptionPointers = e;
		exceptionInfo.ClientPointers = FALSE;

		MINIDUMP_TYPE mdt = (MINIDUMP_TYPE)(MiniDumpWithFullMemory);
		MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hFile, mdt, (e != 0) ? &exceptionInfo : 0, 0, NULL);

		if (hFile)
		{
			CloseHandle(hFile);
			hFile = NULL;
		}
	}
}

LONG WINAPI ExceptionFilter(EXCEPTION_POINTERS* exceptionInfo)
{
	if (IsDebuggerPresent())
		return EXCEPTION_CONTINUE_SEARCH;

	DWORD myThreadId = GetCurrentThreadId();
	DWORD myProcessId = GetCurrentProcessId();

	CFixedString<64> timeStamp;
	timeStamp.MakeDateTime(_T('-'), _T(' '), _T('-'));
	tofstream writeFile(Generic::FormatSafe(_T("%a ExceptionLog.txt"), timeStamp.Get()), std::ofstream::out);

	if (writeFile.is_open())
	{
		CStackWalker sw;
		sw.LoadModules();
		sw.SetOutputStream(&writeFile);

		// 크래쉬 난 스레드 스택 출력
		writeFile << _T("<stack-crash>") << "\n";
	    sw.ShowCallstack();
		writeFile << _T("\n");
		
		// 그외 스레드
		for (int i = 0; i < MAX_THREAD_COUNT; ++i)
		{
			if (myThreadId == g_ThreadIndexToId[i])
				continue;

			HANDLE thread = OpenThread(THREAD_ALL_ACCESS, FALSE, g_ThreadIndexToId[i]);
			if (thread == NULL)
				continue;;

			writeFile << _T("<stack>") << "\n";
			sw.ShowCallstack(thread);
			writeFile << _T("\n");
		}

		// 스레드 가드 출력
		CThreadSafeGuard::DumpAll(writeFile, GetCurrentThreadIndex());

		// 스택 오버플로인경우 쓰레드 생성해서 덤프 생성
		if (exceptionInfo->ExceptionRecord->ExceptionCode == EXCEPTION_STACK_OVERFLOW)
		{
			std::thread thread(MakeDump, exceptionInfo);
			thread.join();
		}
		else
		{
			MakeDump(exceptionInfo);
		}

		writeFile.flush();
		writeFile.close();
	}

	ExitProcess(1);

	return EXCEPTION_EXECUTE_HANDLER;
}