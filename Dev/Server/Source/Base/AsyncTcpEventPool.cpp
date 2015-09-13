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
	int                m_Size;       ///< ������
	volatile long long m_AllocCount; ///< �Ҵ��� Ƚ��


public:
	/// \brief ������
	CPool(int size)
	:
	m_Queue(),
	m_Size(size),
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
			evt = xnew(CAsyncTcpEvent, m_Size);
			InterlockedIncrement64(&m_AllocCount);
		}

		return evt;
	}

	/// \brief ȸ���Ѵ�.
	void Dealloc(CAsyncTcpEvent* evt)
	{
		assert(evt->m_Capacity == m_Size && "event size invalid");
		m_Queue.push(evt);
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
	for (int i = 2; i <= CAsyncTcpEvent::MAX_CAPACITY; i += 2)
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

CAsyncTcpEventPool* g_AsyncTcpEventPool = nullptr;