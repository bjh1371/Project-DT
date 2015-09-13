////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file AsyncTcpSocketPool.h
/// \author bjh1371
/// \date 2015/09/04
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

class CAsyncTcpSocket;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \class CAsyncTcpSocketPool
/// \brief ������ �Ҵ��ϴ� Ǯ
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CAsyncTcpSocketPool
{
private:
	typedef tbb::concurrent_queue<CAsyncTcpSocket*> CSocketQueue;


private:
	CSocketQueue       m_Queue;      ///< ���� ť
	volatile long long m_AllocCount; ///< �Ҵ� ī��Ʈ


public:
	/// \brief ������
	CAsyncTcpSocketPool();
	
	/// \brief �Ҹ���
	~CAsyncTcpSocketPool();
	
	
public:
	/// \brief ���� Pop
	CAsyncTcpSocket* Pop();

	/// \brief ���� Push
	void Push(CAsyncTcpSocket* socket);


public:
	/// \brief �Ҵ��� ���� 
	int GetAllocCount() const { return static_cast<int>(m_AllocCount); }

	/// \brief ��밡���� ����
	int GetFreeCount() const { return static_cast<int>(m_Queue.unsafe_size()); }
};

extern CAsyncTcpSocketPool* g_AsyncTcpSocketPool;