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
	CPeerQueue         m_Queue;      ///< �Ǿ� ��Ȱ�� �ϴ� ť
	CAllocPeerArray    m_PeerArray;  ///< �Ҵ�� �Ǿ���� ���� �ϴ� �迭
	volatile long long m_AllocCount; ///< �Ǿ� �Ҵ� ����
	SOCKET             m_ListenSock; ///< ��������



public:
	/// \brief ������
	CAsyncPeerListener(short port);

	/// \brief �Ҹ���
	virtual ~CAsyncPeerListener();


public:
	/// \brief ������ �Ҵ��Ѵ�.
	void Init(int maxAcceptor);


public:
	/// \brief Peer Pop
	virtual CAsyncTcpPeer* PopPeer();

	/// \brief Peer Push
	void PushPeer(CAsyncTcpPeer* peer);


public:
	/// \brief �Ҵ��� �Ǿ� ����
	int GetAllocCount() const { return static_cast<int>(m_AllocCount); }

	/// \brief ���� ��� ������ �Ǿ� ����
	int GetFreeCount() const { return static_cast<int>(m_Queue.unsafe_size()); }


private:
	/// \brief ���� ����
	virtual CAsyncTcpPeer* CreatePeer() = 0;


public:
	SOCKET GetListenSock() const { return m_ListenSock; }
	void SetListenSock(SOCKET val) { m_ListenSock = val; }
};
