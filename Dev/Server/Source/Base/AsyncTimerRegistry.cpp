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
	CTimerQueue      m_ShortTimer[SHORT_TIMER_ARRAY_LENTGH]; ///< ��ƮŸ�̸� 
	CCriticalSection m_Lock;                                 ///< �ε��� ��
	int              m_Index;                                ///< �ε���


public:
	/// \brief ������
	CShortTimer()
	:
	m_Index(0)
	{
		SafeGuard();
	}

	/// \brief �Ҹ���
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

	/// \brief �߰��Ѵ�.
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
	/// \brief �����Լ�
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
	/// \brief ������
	CLongTimer()
	{
		SafeGuard();
	}

	/// \brief �Ҹ���
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
	/// \brief �߰��Ѵ�.
	void Add(CAsyncTimerEvent* evt)
	{
		SafeGuard();

		m_LongTimerQueue.push(evt);
	}

public:
	/// \brief �����Լ�
	void Heartbeat()
	{
		SafeGuard();

		CAsyncTimerEvent* evt = nullptr;
		size_t size = m_LongTimerQueue.size();
		for (int i = 0; i < size; ++i)
		{
			if (m_LongTimerQueue.try_pop(evt))
			{
				// �ð��� ������ ť��
				if (Clock::GetCurrentMilli() > evt->GetActivation())
				{
					g_AsyncDispatcher->Enqueue(evt->GetSink(), evt);
				}
				// �ȉ����� �ٽ� ����ְ�
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
/// \brief ������
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
/// \brief �Ҹ���
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CAsyncTimerRegistry::~CAsyncTimerRegistry()
{
	SafeGuard();

	Generic::SafeDelete(m_ShortTimer);
	Generic::SafeDelete(m_LongTimer);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Ÿ�̸� �߰��Ѵ�.
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

	// short Ÿ�̸� ����
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Ÿ�̸� �̺�Ʈ
/// \param evt �̺�Ʈ
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