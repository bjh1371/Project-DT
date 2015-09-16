////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file SessionListener.cpp
/// \author 
/// \date 2014.10.28
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "AsyncPeerListener.h"

#include "AsyncDispatcher.h"
#include "AsyncTcpSocketPool.h"
#include "AsyncTcpPeer.h"

#include "Lifespan.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief ������
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CAsyncPeerListener::CAsyncPeerListener(short port)
:
m_Queue(),
m_ListenSock(INVALID_SOCKET),
m_AllocCount(0)
{
	SafeGuard();

	bool success = false;

	do
	{
		m_ListenSock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
		if (m_ListenSock == INVALID_SOCKET)

		{
			TRACE(_T("Listen Sock Wsasocket failed"));
		}

		// ReuseAddr
		int opt = 1;
		if (setsockopt(m_ListenSock, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(int)) != 0)
			break;

		if (!g_AsyncDispatcher->Associate((HANDLE)(m_ListenSock), 0))
			break;

		SOCKADDR_IN addr;
		memset(&addr, 0, sizeof(addr));
		addr.sin_addr.s_addr = htonl(INADDR_ANY);
		addr.sin_family = AF_INET;
		addr.sin_port = htons(port);

		if (bind(m_ListenSock, (SOCKADDR*)&addr, sizeof(addr)) != 0)
			break;

		if (listen(m_ListenSock, 300) != 0)
			break;

		success = true;

	} while (0);

	if (!success)
	{
		TRACE(_T("Listen Failed (Port: %a)"), port);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief �Ҹ���
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CAsyncPeerListener::~CAsyncPeerListener()
{
	SafeGuard();

	// ���� ���� �ݰ�
	if (m_ListenSock != INVALID_SOCKET)
	{
		closesocket(m_ListenSock);
		m_ListenSock = INVALID_SOCKET;
	}

	// �Ҵ�� ��� �Ǿ�� ���� Disconnect
	for (auto itr(m_PeerArray.begin()); itr != m_PeerArray.end(); ++itr)
	{
		CAsyncTcpPeer* peer = *itr;
		peer->PostDisconnect(CAsyncTcpEvent::DR_FORCE);
	}

	// ��� �Ǿ� ����
	for (int delCount = 0; delCount < m_AllocCount;)
	{
		CAsyncTcpPeer* peer = nullptr;
		while (m_Queue.try_pop(peer))
		{
			if (peer)
			{
				xdelete(peer);
				++delCount;
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief ������ �������
/// \param maxSession �ְ� ����
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAsyncPeerListener::Init(int maxAcceptor)
{
	SafeGuard();

	for (int i = 0; i < maxAcceptor; ++i)
	{
		// �ִ� Accept��ŭ ���� ������ Accept�̺�Ʈ ����Ʈ
		CAsyncTcpSocket* socket = g_AsyncTcpSocketPool->Pop();
		socket->PostAccept(this);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief 
/// \return CAsyncTcpPeer*
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CAsyncTcpPeer* CAsyncPeerListener::PopPeer()
{
	SafeGuard();

	CAsyncTcpPeer* peer = nullptr;
	if (!m_Queue.try_pop(peer))
	{
		peer = CreatePeer();
		
		m_PeerArray.push_back(peer);
		peer->SetListener(this);
		
		InterlockedIncrement64(&m_AllocCount);
	}

	return peer;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief 
/// \param CAsyncTcpPeer * peer 
/// \return void
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAsyncPeerListener::PushPeer(CAsyncTcpPeer* peer)
{
   SafeGuard();

	m_Queue.push(peer);
}
