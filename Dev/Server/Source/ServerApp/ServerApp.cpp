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
/// \brief ������
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CServerApp::CServerApp(HINSTANCE hInstance, LPCTSTR title, LPCTSTR guid)
:
m_ServerForm(nullptr)
{
	SafeGuard();

	// ������ �� ����
	m_ServerForm = xnew(CServerForm, hInstance, title, guid);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief �Ҹ���
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

	// �̺�Ʈ Ǯ ����
	g_AsyncTcpEventPool = xnew(CAsyncTcpEventPool);

	// IOCP ����
	g_AsyncDispatcher = xnew(CAsyncDispatcher);
	g_AsyncDispatcher->Start();

	// Ÿ�̸� ����
	g_AsyncTimerRegistry = xnew(CAsyncTimerRegistry);

	// ������ �÷��� ����
	g_AsyncEventSinkRecycler = xnew(CAsyncEventSinkRecycler);
	g_AsyncEventSinkRecycler->Start();

	/// \brief ����Ǯ ����
	g_AsyncTcpSocketPool = xnew(CAsyncTcpSocketPool);

	// Form�� ���������� �ʱ�ȭ �糪?
	if (m_ServerForm->IsGood())
	{
		// ���� �ڿ� ����
		bool success = Setup();

		if (success)
		{
			// Ÿ�̸� ����
			m_StatusTimer = g_AsyncTimerRegistry->AddTimer(this, 500, Clock::BOUNDLESS);

			while (IsAppRunning())
			{
				g_AsyncTimerRegistry->Hearbeat();
			}
		}

		// �ڿ� ����
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

	// ���� ����
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