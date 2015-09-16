////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file AsyncTcpSocketPool.h
/// \author bjh1371
/// \date 2015/09/04
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

class CAsyncTcpSocket;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \class CAsyncTcpSocketPool
/// \brief 소켓을 할당하는 풀
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CAsyncTcpSocketPool
{
private:
	typedef tbb::concurrent_queue<CAsyncTcpSocket*> CSocketQueue;


private:
	CSocketQueue       m_Queue;      ///< 소켓 큐
	volatile long long m_AllocCount; ///< 할당 카운트


public:
	/// \brief 생성자
	CAsyncTcpSocketPool();
	
	/// \brief 소멸자
	~CAsyncTcpSocketPool();
	
	
public:
	/// \brief 소켓 Pop
	CAsyncTcpSocket* Pop();

	/// \brief 소켓 Push
	void Push(CAsyncTcpSocket* socket);


public:
	/// \brief 할당한 숫자 
	int GetAllocCount() const { return static_cast<int>(m_AllocCount); }

	/// \brief 사용가능한 숫자
	int GetFreeCount() const { return static_cast<int>(m_Queue.unsafe_size()); }
};

extern CAsyncTcpSocketPool* g_AsyncTcpSocketPool;