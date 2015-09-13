////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file ThreadSafeGuard.cpp
/// \author bjh1371
/// \date 2015/06/08
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "ThreadSafeGuard.h"

#include "ThreadRegistry.h"

namespace
{
	const int MAX_STACK_COUNT = 255;
	
	struct ENTRY
	{
		const TCHAR* File;
		int          Line;
		const TCHAR* Func;

		ENTRY()
		:
		File(nullptr),
		Line(0),
		Func(nullptr)
		{

		}
	};

	ENTRY g_EntryArray[MAX_THREAD_COUNT][MAX_STACK_COUNT];
	int   g_EntryCount[MAX_THREAD_COUNT] = {0, };
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief 생성자
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CThreadSafeGuard::CThreadSafeGuard(const TCHAR* file, int line, const TCHAR* func, Milli_t checkRunningTime)
:
m_StartTime(Clock::GetCurrentMilli()),
m_CheckRunningTime(checkRunningTime)
{
	Push(file, line, func);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief 소멸자
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CThreadSafeGuard::~CThreadSafeGuard()
{
	Pop();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief 
/// \param const TCHAR * file 
/// \param int line 
/// \param const TCHAR * func 
/// \param int checkRunningTime 
/// \return void
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CThreadSafeGuard::Push(const TCHAR* file, int line, const TCHAR* func)
{
	long tIndex = GetCurrentThreadIndex();

	int stackCount = g_EntryCount[tIndex];
	if (stackCount < MAX_STACK_COUNT)
	{
		ENTRY& entry = g_EntryArray[tIndex][g_EntryCount[tIndex]];
		entry.File = file;
		entry.Func = func;
		entry.Line = line;
		
		++g_EntryCount[GetCurrentThreadIndex()];
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief 
/// \return void
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CThreadSafeGuard::Pop()
{
	long tIndex = GetCurrentThreadIndex();
	--g_EntryCount[tIndex];

	Milli_t runningTime = Clock::GetCurrentMilli() - m_StartTime;
	if (m_CheckRunningTime < runningTime)
	{
		const ENTRY& entry = g_EntryArray[tIndex][g_EntryCount[tIndex]];
		TRACE(_T("slow running function : %a millisecond time func (File : %a, Line : %a, Func : %a)"), runningTime, entry.File, entry.Line, entry.Func);
	}
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief 덤프
/// \param tofstream * ofstream 출력할 스트림
/// \param long crashIndex 크래시 난 스레드 인덱스
/// \return void
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CThreadSafeGuard::DumpAll(tofstream& ofstream, long crashIndex)
{
	for (int i = 0; i < MAX_THREAD_COUNT; ++i)
	{
		if (0 < g_EntryCount[i])
		{
			if (crashIndex == i)
			{
				ofstream << _T("ThreadGuard") << i<<_T("<stack-crash>")<<_T("\n");
			}
			else
			{
				ofstream << _T("ThreadGuard") << i<<_T("\n");
			}
			for (int j = g_EntryCount[i] - 1; j >= 0; --j)
			{
				ENTRY& entry = g_EntryArray[i][j];
				ofstream << entry.Func << _T(" File : ") << entry.File << _T(" Line : ") << entry.Line << _T("\n");
			}
			ofstream << _T("\n\n");
		}
	}
}
