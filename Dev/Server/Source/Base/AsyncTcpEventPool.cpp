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

	CEventQueue        m_Queue;      ///< �̺�Ʈ ť
	int                m_Capacity;   ///< ������
	volatile long long m_AllocCount; ///< �Ҵ��� Ƚ��


public:
	/// \brief ������
	CPool(int capacity)
	:
	m_Queue(),
	m_Capacity(capacity),
	m_AllocCount(0)
	{

	}

	/// \brief �Ҹ���
	~CPool()
	{
		// ��� �̺�Ʈ ����
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
	/// \brief �Ҵ��Ѵ�.
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

	/// \brief ȸ���Ѵ�.
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
	/// \brief ��� ������ �̺�Ʈ ���� ��ȯ
	int GetFreeCount()
	{
		return static_cast<int>(m_Queue.unsafe_size());
	}

	/// \brief ��ü �Ҵ��� ���� ��ȯ
	int GetTotalCount()
	{
		return static_cast<int>(m_AllocCount);
	}
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief ������
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CAsyncTcpEventPool::CAsyncTcpEventPool()
:
m_PoolMap()
{
	// Ǯ ����
	for (int i = CAsyncTcpEvent::MIN_CAPACITY; i <= CAsyncTcpEvent::MAX_CAPACITY; i += 2)
	{
		CPool* pool = xnew(CPool, i);
		m_PoolMap.insert(CPoolMap::value_type(i, pool));
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief �Ҹ���
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CAsyncTcpEventPool::~CAsyncTcpEventPool()
{
	for (auto& pool : m_PoolMap)
	{
		xdelete(pool.second);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief TcpEvent�� �Ҵ��Ѵ�.
/// \param int size ������
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
/// \brief TcpEvent�� ȸ���Ѵ�.
/// \param CAsyncTcpEvent * evt ȸ���� �̺�Ʈ 
/// \return void
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAsyncTcpEventPool::Dealloc(CAsyncTcpEvent* evt)
{
	// ���� ��Ű��
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
/// \brief Ư�� ������ �̺�Ʈ ť�� ��� ������ ���ڸ� ��ȯ�Ѵ�.
/// \param int capacity �ش� ������
/// \return int ��밡���� ����
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
/// \brief Ư�� ������ �̺�Ʈ ť�� ��ü �Ҵ� �� ���ڸ� ��ȯ�Ѵ�.
/// \param int capacity �ش� ������
/// \return int �Ҵ��� ����
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