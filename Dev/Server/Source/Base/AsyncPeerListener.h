////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file CAsyncPeerListener.h
/// \author 
/// \date 2014.10.28
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

class CAsyncTcpPeer;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \class CAsyncPeerListener
/// \brief 
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CAsyncPeerListener
{
private:
	typedef tbb::concurrent_queue<CAsyncTcpPeer*> CPeerQueue;
	typedef tbb::concurrent_vector<CAsyncTcpPeer*> CAllocPeerArray;


private:
	CPeerQueue         m_Queue;      ///< 피어 재활용 하는 큐
	CAllocPeerArray    m_PeerArray;  ///< 할당된 피어들을 관리 하는 배열
	volatile long long m_AllocCount; ///< 피어 할당 숫자
	SOCKET             m_ListenSock; ///< 리슨소켓



public:
	/// \brief 생성자
	CAsyncPeerListener(short port);

	/// \brief 소멸자
	virtual ~CAsyncPeerListener();


public:
	/// \brief 세션을 할당한다.
	void Init(int maxAcceptor);


public:
	/// \brief Peer Pop
	virtual CAsyncTcpPeer* PopPeer();

	/// \brief Peer Push
	void PushPeer(CAsyncTcpPeer* peer);


public:
	/// \brief 할당한 피어 숫자
	int GetAllocCount() const { return static_cast<int>(m_AllocCount); }

	/// \brief 현재 사용 가능한 피어 숫자
	int GetFreeCount() const { return static_cast<int>(m_Queue.unsafe_size()); }


private:
	/// \brief 세션 생성
	virtual CAsyncTcpPeer* CreatePeer() = 0;


public:
	SOCKET GetListenSock() const { return m_ListenSock; }
	void SetListenSock(SOCKET val) { m_ListenSock = val; }
};
