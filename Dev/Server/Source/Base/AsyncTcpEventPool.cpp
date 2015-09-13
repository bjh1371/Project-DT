////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file AsyncTcpEventPool.cpp
/// \author bjh1371
/// \date 2015/09/13
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "AsyncTcpEventPool.h"

#include "AsyncTcpEvent.h"
#include "Math.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Pool
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CAsyncTcpEventPool::CPool
{
private:
	typedef tbb::concurrent_queue<CAsyncTcpEvent*> CEventQueue;

	CEventQueue        m_Queue;      ///< 이벤트 큐
	int                m_Size;       ///< 사이즈
	volatile long long m_AllocCount; ///< 할당한 횟수


public:
	/// \brief 생성자
	CPool(int size)
	:
	m_Queue(),
	m_Size(size),
	m_AllocCount(0)
	{

	}

	/// \brief 소멸자
	~CPool()
	{
		// 모든 이벤트 정리
		for (int delCount = 0; delCount < m_AllocCount;)
		{
			CAsyncTcpEvent* evt = nullptr;
			while (m_Queue.try_pop(evt))
			{
				if (evt)
				{
					xdelete(evt);
					++delCount;
				}
			}
		}
	}


public:
	/// \brief 할당한다.
	CAsyncTcpEvent* Alloc()
	{
		CAsyncTcpEvent* evt = nullptr;
		if (!m_Queue.try_pop(evt))
		{
			evt = xnew(CAsyncTcpEvent, m_Size);
			InterlockedIncrement64(&m_AllocCount);
		}

		return evt;
	}

	/// \brief 회수한다.
	void Dealloc(CAsyncTcpEvent* evt)
	{
		assert(evt->m_Capacity == m_Size && "event size invalid");
		m_Queue.push(evt);
	}
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief 생성자
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CAsyncTcpEventPool::CAsyncTcpEventPool()
:
m_PoolMap()
{
	// 풀 생성
	for (int i = 2; i <= CAsyncTcpEvent::MAX_CAPACITY; i += 2)
	{
		CPool* pool = xnew(CPool, i);
		m_PoolMap.insert(CPoolMap::value_type(i, pool));
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief 소멸자
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CAsyncTcpEventPool::~CAsyncTcpEventPool()
{
	for (auto& pool : m_PoolMap)
	{
		xdelete(pool.second);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief TcpEvent를 할당한다.
/// \param int size 사이즈
/// \return void
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CAsyncTcpEvent* CAsyncTcpEventPool::Alloc(int size)
{
	size = MathUtil::GetNearestPowerOfTwo(size);

	CAsyncTcpEvent* evt = nullptr;

	auto itr = m_PoolMap.find(size);
	if (itr != m_PoolMap.end())
	{
		CPool* pool = itr->second;
		evt = pool->Alloc();
	}
	else
	{
		assert(false && "invalid alloc size");
	}

	return evt;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief TcpEvent를 회수한다.
/// \param CAsyncTcpEvent * evt 회수할 이벤트 
/// \return void
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAsyncTcpEventPool::Dealloc(CAsyncTcpEvent* evt)
{
	// 리셋 시키고
	evt->Reset();

	auto itr = m_PoolMap.find(evt->m_Capacity);
	if (itr != m_PoolMap.end())
	{
		CPool* pool = itr->second;
		pool->Dealloc(evt);
	}
	else
	{
		assert(false && "invalid alloc size");
	}
}

CAsyncTcpEventPool* g_AsyncTcpEventPool = nullptr;