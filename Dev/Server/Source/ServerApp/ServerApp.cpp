////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file ServerApp.cpp
/// \author bjh1371
/// \date 2015/07/03
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "ServerApp.h"

#include "ServerForm.h"

#include "Base/AsyncDispatcher.h"
#include "Base/AsyncEventSinkRecycler.h"
#include "Base/AsyncTcpEvent.h"
#include "Base/AsyncTcpEventPool.h"
#include "Base/AsyncTcpSocketPool.h"
#include "Base/AsyncTimerRegistry.h"
#include "Base/FixedString.h"
#include "Base/Lifespan.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief 생성자
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CServerApp::CServerApp(HINSTANCE hInstance, LPCTSTR title, LPCTSTR guid)
:
m_ServerForm(nullptr)
{
	SafeGuard();

	// 윈도우 폼 생성
	m_ServerForm = xnew(CServerForm, hInstance, title, guid);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief 소멸자
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CServerApp::~CServerApp()
{
	SafeGuard();

	Generic::SafeDelete(m_ServerForm);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief 
/// \return void
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CServerApp::Start()
{
	SafeGuard();

	// 이벤트 풀 생성
	g_AsyncTcpEventPool = xnew(CAsyncTcpEventPool);

	// IOCP 생성
	g_AsyncDispatcher = xnew(CAsyncDispatcher);
	g_AsyncDispatcher->Start();

	// 타이머 생성
	g_AsyncTimerRegistry = xnew(CAsyncTimerRegistry);

	// 가비지 컬렉터 생성
	g_AsyncEventSinkRecycler = xnew(CAsyncEventSinkRecycler);
	g_AsyncEventSinkRecycler->Start();

	/// \brief 소켓풀 생성
	g_AsyncTcpSocketPool = xnew(CAsyncTcpSocketPool);

	// Form이 정상적으로 초기화 됬나?
	if (m_ServerForm->IsGood())
	{
		// 각종 자원 셋팅
		bool success = Setup();

		if (success)
		{
			// 타이머 시작
			m_StatusTimer = g_AsyncTimerRegistry->AddTimer(this, 500, Clock::BOUNDLESS);

			while (IsAppRunning())
			{
				g_AsyncTimerRegistry->Hearbeat();
			}
		}

		// 자원 해제
		TearDown();
	}

	Generic::SafeDelete(g_AsyncTimerRegistry);
	Generic::SafeDelete(g_AsyncDispatcher);
	Generic::SafeDelete(g_AsyncTcpSocketPool);

	g_AsyncEventSinkRecycler->Stop();
	g_AsyncEventSinkRecycler->Join();
	Generic::SafeDelete(g_AsyncEventSinkRecycler);

	Generic::SafeDelete(g_AsyncTcpEventPool);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief 
/// \param LPCTSTR key 
/// \param int value 
/// \return void
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CServerApp::PostItem(LPCTSTR key, int value)
{
	SafeGuard();

	if (m_ServerForm)
	{
		m_ServerForm->PostItem(key, value);
	}
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief 
/// \param LPCTSTR key 
/// \param LPCTSTR value 
/// \return void
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CServerApp::PostItem(LPCTSTR key, LPCTSTR value)
{
	SafeGuard();

	if (m_ServerForm)
	{
		m_ServerForm->PostItem(key, value);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief 
/// \return void
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CServerApp::ReportStatus()
{
	SafeGuard();

	// 상태 보고
	PostItem(_T("AllocTcpSocket"), Generic::FormatSafe(_T("%a / %a"), g_AsyncTcpSocketPool->GetFreeCount(), g_AsyncTcpSocketPool->GetAllocCount()));
	PostItem(_T("GarbageSinkCount"), g_AsyncEventSinkRecycler->GetCount());

	CFixedString<64> key;
	CFixedString<128> values;
	CFixedString<64> v;
	for (int capacity = CAsyncTcpEvent::MIN_CAPACITY; capacity <= CAsyncTcpEvent::MAX_CAPACITY;)
	{
		int maxCapacity = capacity<<2;
		maxCapacity = maxCapacity < CAsyncTcpEvent::MAX_CAPACITY ? maxCapacity : CAsyncTcpEvent::MAX_CAPACITY;
		key.FormatSafe(_T("AsyncTcpEvent ~ %a"), maxCapacity);
		for (int i = 0; i < 3 && capacity <= CAsyncTcpEvent::MAX_CAPACITY; capacity *= 2, ++i)
		{
			if (capacity <= CAsyncTcpEvent::MAX_CAPACITY)
			{
				v.FormatSafe(_T("%a =  %a / %a,  "), capacity, g_AsyncTcpEventPool->GetFreeCount(capacity), g_AsyncTcpEventPool->GetTotalCount(capacity));
				values.Append(v);
			}
			else
				break;
		}

		PostItem(key, values);
		values.Clear();
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief 
/// \param CAsyncTimerEvent * evt 
/// \return void
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CServerApp::OnTimer(CAsyncTimerEvent* evt)
{
	SafeGuard();

	if (evt == m_StatusTimer)
	{
		ReportStatus();
	}
}