////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file CAsyncRecycler.cpp
/// \author 
/// \date 2014.11.3
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "AsyncEventSinkRecycler.h"

#include "AsyncTimerEvent.h"
#include "AsyncTimerRegistry.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief 생성자
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CAsyncEventSinkRecycler::CAsyncEventSinkRecycler()
:
m_Queue(),
m_LifeSpan(true)
{
	SafeGuard();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief 소멸자
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CAsyncEventSinkRecycler::~CAsyncEventSinkRecycler()
{
	SafeGuard();

	CAsyncEventSink* sink = nullptr;
	while (m_Queue.try_pop(sink))
	{
		sink->Destroy();
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief 
/// \return void
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAsyncEventSinkRecycler::Stop()
{
	m_LifeSpan = false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief 리사이클러에 추가한다.
/// \param session 추가할 세션
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAsyncEventSinkRecycler::Add(CAsyncEventSink* sink)
{
	SafeGuard();
	
	if (sink->ExChangePendingDeletion(true) == false)
	{
		m_Queue.push(sink);
	}
	else
	{
		TRACE(_T("duplicate deletion"));
	}
	
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief 현재 재활용 큐에 있는 EventSink숫자를 반환한다.
/// \return int 큐에있는 숫자
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CAsyncEventSinkRecycler::GetCount()
{
	return static_cast<int>(m_Queue.unsafe_size());
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief 
/// \return void
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAsyncEventSinkRecycler::ThreadMain()
{
	while (m_LifeSpan)
	{
		size_t size = m_Queue.unsafe_size();
		for (int i = 0; i < size; ++i)
		{
			CAsyncEventSink* sink = nullptr;
			if (m_Queue.try_pop(sink))
			{
				if (sink->IsSafeToDelete())
				{
					sink->Destroy();
				}
				else
				{
					m_Queue.push(sink);
				}
			}
			else
				break;
		}

		Sleep(1000);
	}
}

CAsyncEventSinkRecycler* g_AsyncEventSinkRecycler = nullptr;

