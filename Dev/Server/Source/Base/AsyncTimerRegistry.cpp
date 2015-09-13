////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file TimerRegistry.cpp
/// \author 
/// \date 2014.11.4
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "AsyncTimerRegistry.h"

#include "AsyncDispatcher.h"
#include "AsyncEventSink.h"
#include "AsyncTimerEvent.h"
#include "Lockable.h"
#include "ThreadRegistry.h"

namespace
{
	const int SHORT_TIMER_INTERVAL = 100;

	const int SHORT_TIMER_MAXIMUM = 100000;

	const int SHORT_TIMER_ARRAY_LENTGH = SHORT_TIMER_MAXIMUM / SHORT_TIMER_INTERVAL + 1;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \class CShortTimer
/// \brief 
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CAsyncTimerRegistry::CShortTimer
{
private:
	typedef tbb::concurrent_queue<CAsyncTimerEvent*> CTimerQueue;
	

private:
	CTimerQueue      m_ShortTimer[SHORT_TIMER_ARRAY_LENTGH]; ///< 쇼트타이머 
	CCriticalSection m_Lock;                                 ///< 인덱스 락
	int              m_Index;                                ///< 인덱스


public:
	/// \brief 생성자
	CShortTimer()
	:
	m_Index(0)
	{
		SafeGuard();
	}

	/// \brief 소멸자
	~CShortTimer()
	{
		SafeGuard();

		CAsyncTimerEvent* evt = nullptr;
		for (int i = 0; i < SHORT_TIMER_ARRAY_LENTGH; ++i)
		{
			CTimerQueue& timerQueue = m_ShortTimer[i];
			while (timerQueue.try_pop(evt))
			{
				evt->GetSink()->DecreaseRefCount(CAsyncEventSink::REFCOUNT_TIMER);
				xdelete(evt);
			}
		}
	}

	/// \brief 추가한다.
	void Add(CAsyncTimerEvent* evt)
	{
		SafeGuard();

		int index = 0;
		m_Lock.Synchronize([&]
		{
			index = ((m_Index + (evt->GetInterval() / SHORT_TIMER_INTERVAL)) - 1) % SHORT_TIMER_ARRAY_LENTGH;
		});
		
		CTimerQueue& timerQueue = m_ShortTimer[index];
		timerQueue.push(evt);
	}


public:
	/// \brief 폴링함수
	void Heartbeat()
	{
		SafeGuard();

		int index = m_Index;
		m_Lock.Synchronize([&]
		{
			m_Index = ++m_Index % SHORT_TIMER_ARRAY_LENTGH;
		});
		
		CTimerQueue& timerQueue = m_ShortTimer[index];
		CAsyncTimerEvent* evt = nullptr;

		while (timerQueue.try_pop(evt))
		{
			g_AsyncDispatcher->Enqueue(evt->GetSink(), evt);
		}
	}
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \class CLongTimer
/// \brief 
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CAsyncTimerRegistry::CLongTimer
{
private:
	struct COMPARE
	{
		bool operator()(const CAsyncTimerEvent* lhs, const CAsyncTimerEvent* rhs)
		{
			return lhs->GetActivation() > rhs->GetActivation();
		}
	};

	typedef tbb::concurrent_priority_queue<CAsyncTimerEvent*, COMPARE> CLongTimerQueue;


private:
	CLongTimerQueue m_LongTimerQueue;


public:
	/// \brief 생성자
	CLongTimer()
	{
		SafeGuard();
	}

	/// \brief 소멸자
	~CLongTimer()
	{
		SafeGuard();

		CAsyncTimerEvent* evt = nullptr;
		while (m_LongTimerQueue.try_pop(evt))
		{
			evt->GetSink()->DecreaseRefCount(CAsyncEventSink::REFCOUNT_TIMER);
			xdelete(evt);
		}
	}

public:
	/// \brief 추가한다.
	void Add(CAsyncTimerEvent* evt)
	{
		SafeGuard();

		m_LongTimerQueue.push(evt);
	}

public:
	/// \brief 폴링함수
	void Heartbeat()
	{
		SafeGuard();

		CAsyncTimerEvent* evt = nullptr;
		size_t size = m_LongTimerQueue.size();
		for (int i = 0; i < size; ++i)
		{
			if (m_LongTimerQueue.try_pop(evt))
			{
				// 시간이 됬으면 큐잉
				if (Clock::GetCurrentMilli() > evt->GetActivation())
				{
					g_AsyncDispatcher->Enqueue(evt->GetSink(), evt);
				}
				// 안됬으면 다시 집어넣고
				else
				{
					m_LongTimerQueue.push(evt);
					break;
				}
			}
			else
				break;
		}
	}

};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief 생성자
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CAsyncTimerRegistry::CAsyncTimerRegistry()
:
m_ShortTimer(nullptr),
m_LongTimer(nullptr)
{
	SafeGuard();

	m_ShortTimer = xnew(CShortTimer);
	m_LongTimer = xnew(CLongTimer);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief 소멸자
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CAsyncTimerRegistry::~CAsyncTimerRegistry()
{
	SafeGuard();

	Generic::SafeDelete(m_ShortTimer);
	Generic::SafeDelete(m_LongTimer);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief 타이머 추가한다.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CAsyncTimerEvent* CAsyncTimerRegistry::AddTimer(CAsyncEventSink* sink, Milli_t interval, Milli_t expiration)
{
	SafeGuard();

	CAsyncTimerEvent* evt = xnew(CAsyncTimerEvent);
	evt->IncreaseRefCount();

	evt->SetSink(sink);
	evt->SetInterval(interval);
	evt->SetExpiration(expiration >= Clock::BOUNDLESS ? Clock::BOUNDLESS : expiration + Clock::GetCurrentMilli());
	evt->SetActivation(Clock::GetCurrentMilli() + interval);

	sink->IncreaseRefCount(CAsyncEventSink::REFCOUNT_TIMER);

	if (interval < SHORT_TIMER_MAXIMUM)
	{
		m_ShortTimer->Add(evt);
	}
	else
	{
		m_LongTimer->Add(evt);
	}

	return evt;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief 
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAsyncTimerRegistry::Hearbeat()
{
	SafeGuard();

	m_ShortTimer->Heartbeat();
	m_LongTimer->Heartbeat();

	// short 타이머 간격
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief 타이머 이벤트
/// \param evt 이벤트
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAsyncTimerRegistry::Add(CAsyncTimerEvent* evt)
{
    SafeGuard();
 
	if (evt->GetInterval() < SHORT_TIMER_MAXIMUM)
	{
		m_ShortTimer->Add(evt);
	}
	else
	{
		m_LongTimer->Add(evt);
	}
}

CAsyncTimerRegistry* g_AsyncTimerRegistry = nullptr;