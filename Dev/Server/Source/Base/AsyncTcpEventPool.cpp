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
	int                m_Capacity;   ///< 사이즈
	volatile long long m_AllocCount; ///< 할당한 횟수


public:
	/// \brief 생성자
	CPool(int capacity)
	:
	m_Queue(),
	m_Capacity(capacity),
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
			evt = xnew(CAsyncTcpEvent, m_Capacity);
			InterlockedIncrement64(&m_AllocCount);
		}

		return evt;
	}

	/// \brief 회수한다.
	void Dealloc(CAsyncTcpEvent* evt)
	{
		if (evt->m_Capacity == m_Capacity)
		{
			m_Queue.push(evt);
		}
		else
		{
			assert(false && "event size invalid");
		}
	}


public:
	/// \brief 사용 가능한 이벤트 숫자 반환
	int GetFreeCount()
	{
		return static_cast<int>(m_Queue.unsafe_size());
	}

	/// \brief 전체 할당한 숫자 반환
	int GetTotalCount()
	{
		return static_cast<int>(m_AllocCount);
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
	for (int i = CAsyncTcpEvent::MIN_CAPACITY; i <= CAsyncTcpEvent::MAX_CAPACITY; i += 2)
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
	size = MathUtil::GetNearestPowerOfTwo(size) < CAsyncTcpEvent::MIN_CAPACITY ? CAsyncTcpEvent::MIN_CAPACITY : size;

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

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief 특정 사이즈 이벤트 큐의 사용 가능한 숫자를 반환한다.
/// \param int capacity 해당 사이즈
/// \return int 사용가능한 숫자
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CAsyncTcpEventPool::GetFreeCount(int capacity)
{
	int freeCount = 0;
	
	auto itr(m_PoolMap.find(capacity));
	if (itr != m_PoolMap.end())
	{
		CPool* pool = itr->second;
		freeCount = pool->GetFreeCount();
	}

	return freeCount;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief 특정 사이즈 이벤트 큐의 전체 할당 한 숫자를 반환한다.
/// \param int capacity 해당 사이즈
/// \return int 할당한 숫자
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CAsyncTcpEventPool::GetTotalCount(int capacity)
{
	int totalCount = 0;

	auto itr(m_PoolMap.find(capacity));
	if (itr != m_PoolMap.end())
	{
		CPool* pool = itr->second;
		totalCount = pool->GetTotalCount();
	}

	return totalCount;
}

CAsyncTcpEventPool* g_AsyncTcpEventPool = nullptr;