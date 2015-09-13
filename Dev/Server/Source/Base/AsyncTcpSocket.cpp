////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file AsyncTcpSocket.cpp
/// \author bjh1371
/// \date 2015/07/08
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "AsyncTcpSocket.h"

#include "AsyncDispatcher.h"
#include "AsyncEventSinkRecycler.h"
#include "AsyncPeerListener.h"
#include "AsyncTcpPeer.h"
#include "AsyncTcpSocketPool.h"
#include "Lifespan.h"

#pragma comment(lib, "ws2_32")

namespace
{
	LPFN_ACCEPTEX             g_FnAcceptEx = nullptr;
	LPFN_GETACCEPTEXSOCKADDRS g_FnGetAcceptExSockAddrs = nullptr;
	LPFN_DISCONNECTEX         g_FnDisconnectEx = nullptr;
	LPFN_CONNECTEX            g_FnConnectEx = nullptr;

	// Ȯ�� �Լ� �ε�
	void LoadExtensionFn()
	{
		SafeGuard();

		// Ȯ�� �Լ� �ε�� ����
		SOCKET loadFnSocket = INVALID_SOCKET;
		loadFnSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);

		if (loadFnSocket == INVALID_SOCKET)

		{
			TRACE(_T("Sock Wsasocket failed"));
		}

		void** lpfn = nullptr;
		GUID guid;
		DWORD bytes = 0;
		for (int i = 0; i < 4; ++i)
		{
			switch (i)
			{
			case 0:
				lpfn = reinterpret_cast<void**>(&g_FnAcceptEx);
				guid = WSAID_ACCEPTEX;
				break;
			case 1:
				lpfn = reinterpret_cast<void**>(&g_FnConnectEx);
				guid = WSAID_CONNECTEX;
				break;
			case 2:
				lpfn = reinterpret_cast<void**>(&g_FnDisconnectEx);
				guid = WSAID_DISCONNECTEX;
				break;
			case 3:
				lpfn = reinterpret_cast<void**>(&g_FnGetAcceptExSockAddrs);
				guid = WSAID_GETACCEPTEXSOCKADDRS;
				break;
			}

			if (WSAIoctl(loadFnSocket, SIO_GET_EXTENSION_FUNCTION_POINTER, &guid, sizeof(GUID), lpfn, sizeof(void*), &bytes, 0, 0) != 0)
			{
				TRACE(_T("Load Extention Function Failed"));
				break;
			}
		}

		// Ȯ�� �Լ� �ε�� ���� �ݱ�
		if (loadFnSocket != INVALID_SOCKET)
		{
			closesocket(loadFnSocket);
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief ������
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CAsyncTcpSocket::CAsyncTcpSocket()
:
m_Addr(),
m_Peer(nullptr),
m_Listener(nullptr),
m_SocketState(SOCKET_STATE_FREE),
m_ForceCloseTick(Clock::BOUNDLESS),
m_Socket(INVALID_SOCKET),
m_Connect(false),
m_Bind(false)
{
	SafeGuard();

	m_Socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (m_Socket != INVALID_SOCKET)
	{
		// IOCP�� ����
		AssociateIocp();
	}
	else
	{
		TRACE(_T("WSASocket Error"));
	}

	memset(&m_Addr, 0, sizeof(m_Addr));
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief �Ҹ���
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CAsyncTcpSocket::~CAsyncTcpSocket()
{
	SafeGuard();

	if (m_Socket != INVALID_SOCKET)
	{
		if (closesocket(m_Socket) == 0)
		{
			m_Socket = INVALID_SOCKET;
		}
		else
		{
			TRACE(_T("CloseSocket Error : %a"), WSAGetLastError());
		}
	}
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief �Ǿ�� ������ �����Ѵ�.
/// \param CAsyncTcpPeer * peer ������ �Ǿ�
/// \return void
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAsyncTcpSocket::SetPeer(CAsyncTcpPeer* peer)
{
	if (peer)
	{
		m_Peer = peer;
		m_Peer->SetSocket(this);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief ���� ������ ����
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAsyncTcpSocket::ForceClose()
{
	SafeGuard();

	if (m_Socket != INVALID_SOCKET)
	{
		// ���� ���� ���·� �ٲٰ�
		m_SocketState = SOCKET_STATE_FORCECLOSED;
		
		// ���� ����
		if (closesocket(m_Socket) == 0)
		{
			m_Socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
			m_Peer = nullptr;
			m_Bind = false;
			m_Connect = false;

			// ������ ���� ���������Ƿ� Iocp�� ����
			AssociateIocp();
		}
		else
		{
			TRACE(_T("closeSocket error : %a"), WSAGetLastError());
		}
	}
	else
	{
		assert(false && "socket state invalid");
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief 
/// \param CAsyncTcpEvent * evt 
/// \return void
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAsyncTcpSocket::PostSend(CAsyncTcpEvent* evt, int totalBytes)
{
	SafeGuard();

	// �����߰� Disconnect �̺�Ʈ�� ����Ʈ���� �ʾ����� Send�Ѵ�.
	if (m_Connect)
	{
		IncreaseRefCount(CAsyncEventSink::REFCOUNT_SEND);
		evt->IncreaseRefCount();

		evt->m_OpType = CAsyncTcpEvent::OP_SEND;
		evt->m_TotalBytes = totalBytes;
		evt->m_WsaBuf.buf = evt->m_Buf;
		evt->m_WsaBuf.len = totalBytes;

		DWORD sentbytes;
		DWORD flag = 0;
		int result = WSASend(m_Socket, &(evt->m_WsaBuf), 1, &sentbytes, flag, reinterpret_cast<LPOVERLAPPED>(evt->GetTag()), NULL);
		if (result != ERROR_SUCCESS && WSAGetLastError() != WSA_IO_PENDING)
		{
			DecreaseRefCount(CRefCountable::REFCOUNT_SEND);
			m_Peer->DecreaseRefCount(CAsyncEventSink::REFCOUNT_SEND);

			if (evt->DecreaseRefCount() == 0)
			{
				CAsyncTcpEvent::Dealloc(evt);
			}
			
			PostDisconnect(CAsyncTcpEvent::DR_SEND_FAIL);
		}
	}
	// Peer���� �ø��� ���� ����ī��Ʈ ����
	else
	{
		m_Peer->DecreaseRefCount(CRefCountable::REFCOUNT_SEND);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief 
/// \param CAsyncTcpEvent * evt 
/// \param int offset 
/// \return void
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAsyncTcpSocket::PostRecv(CAsyncTcpEvent* evt, int offset /*= 0*/)
{
	SafeGuard();

	if (m_Connect)
	{
		IncreaseRefCount(CAsyncEventSink::REFCOUNT_RECV);
		evt->IncreaseRefCount();

		DWORD recvbytes;
		DWORD flag = 0;

		evt->m_OpType = CAsyncTcpEvent::OP_RECV;
		evt->m_TotalBytes = offset;
		evt->m_WsaBuf.buf = evt->m_Buf + offset;
		evt->m_WsaBuf.len = evt->m_Capacity - offset;

		int result = WSARecv(m_Socket, &(evt->m_WsaBuf), 1, &recvbytes, &flag, reinterpret_cast<LPOVERLAPPED>(evt->GetTag()), NULL);
		if (result == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING)
		{
			DecreaseRefCount(CRefCountable::REFCOUNT_RECV);
			m_Peer->DecreaseRefCount(CAsyncEventSink::REFCOUNT_RECV);
			
			if (evt->DecreaseRefCount() == 0)
			{
				CAsyncTcpEvent::Dealloc(evt);
			}

			PostDisconnect(CAsyncTcpEvent::DR_RECV_FAIL);
		}
	}
	// Peer���� �ø��� ���� ����ī��Ʈ ����
	else
	{
		m_Peer->DecreaseRefCount(CRefCountable::REFCOUNT_RECV);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief 
/// \param SOCKET listenSock 
/// \return bool
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CAsyncTcpSocket::PostAccept(CAsyncPeerListener* listener, CAsyncTcpEvent* evt)
{
	SafeGuard();

	m_Listener = listener;

	assert(m_Connect == false && "connect state must not connected");
	assert(m_SocketState == SOCKET_STATE_FREE && "socket state must free");

	bool success = true;
	
	// �̺�Ʈ�� ������ �ٷ� ���
	evt = evt ? evt : CAsyncTcpEvent::Alloc(CAsyncTcpEvent::ACCEPT_BUF_SIZE);

	evt->m_OpType = CAsyncTcpEvent::OP_ACCEPT;
	evt->m_Owner = this;

	// ���� ����ī��Ʈ ����
	IncreaseRefCount(CAsyncEventSink::REFCOUNT_INIT);
	evt->IncreaseRefCount();

	DWORD bytesReceived = 0;
	if (CAsyncTcpSocket::AcceptEx(
		m_Listener->GetListenSock(),
		m_Socket, evt->m_Buf, 0,
		sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16,
		&bytesReceived, reinterpret_cast<LPOVERLAPPED>(evt->GetTag())))
	{
		// true�� ��ȯ�ϸ� �ٷ� �����ߴٴ� ��
		// �� ��쿡�� Iocp���� �Ϸ� �ݹ��� �Ѿ���� �ʴ´�.
		g_AsyncDispatcher->Enqueue(this, evt);
	}
	else
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			int error = WSAGetLastError();

			TRACE(_T("Post Accept Error %a"), WSAGetLastError());

			if (evt->DecreaseRefCount() == 0)
			{
				CAsyncTcpEvent::Dealloc(evt);
			}

			DecreaseRefCount(CRefCountable::REFCOUNT_INIT);
			ForcePush();
			success = false;
		}
	}
	

	return success;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief 
/// \param const TCHAR * address 
/// \param short port 
/// \return void
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAsyncTcpSocket::PostConnect(LPCTSTR address, short port)
{
	SafeGuard();

	assert(m_Connect == false && "connect state must not connected");

	CAsyncTcpEvent* evt = CAsyncTcpEvent::Alloc(0);
	evt->m_OpType = CAsyncTcpEvent::OP_CONNECT;

	// ���� ����ī��Ʈ ����
	IncreaseRefCount(CAsyncEventSink::REFCOUNT_INIT);
	evt->IncreaseRefCount();

	// family�� �����ϰ� 0���� �ʱ�ȭ �ؼ� bind�Ѵ�.
	m_Addr.sin_family = AF_INET;
	m_Addr.sin_port = htons(0);
	m_Addr.sin_addr.s_addr = htonl(INADDR_ANY);


	if (!m_Bind && bind(m_Socket, (SOCKADDR*)&m_Addr, sizeof(m_Addr)) != 0)
	{
		TRACE(_T("bind error : %a"), WSAGetLastError());
	}
	// Bind ����
	else
	{
		m_Bind = true;
	}

	InetPton(AF_INET, address, &m_Addr.sin_addr.s_addr);
	m_Addr.sin_port = htons(port);

	DWORD bytessent = 0;
	if (CAsyncTcpSocket::ConnectEx(m_Socket, (SOCKADDR*)&m_Addr, sizeof(m_Addr), NULL, 0, &bytessent, reinterpret_cast<LPOVERLAPPED>(evt->GetTag())))
	{
		// true�� ��ȯ�ϸ� �ٷ� �����ߴٴ� ��
		// �� ��쿡�� Iocp���� �Ϸ� �ݹ��� �Ѿ���� �ʴ´�.
		g_AsyncDispatcher->Enqueue(this, evt);
	}
	else
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			if (evt->DecreaseRefCount() == 0)
			{
				CAsyncTcpEvent::Dealloc(evt);
			}

			DecreaseRefCount(CRefCountable::REFCOUNT_INIT);

			ForcePush();

			m_Peer->DecreaseRefCount(CRefCountable::REFCOUNT_CONNECT);
			m_Peer->OnDisconnected();

			TRACE(_T("post connect error %a"), WSAGetLastError());
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief 
/// \param CAsyncTcpEvent::DisconnectReason dr 
/// \return void
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAsyncTcpSocket::PostDisconnect(CAsyncTcpEvent::DisconnectReason dr)
{
	SafeGuard();

	if (m_Connect.exchange(false) == true)
	{
		CAsyncTcpEvent* evt = CAsyncTcpEvent::Alloc(0);
		evt->m_DisconReason = dr;
		evt->m_OpType = CAsyncTcpEvent::OP_DISCONNECT;
		
		evt->IncreaseRefCount();

		if (m_Peer)
		{
			// �� �ܰ迡�� Peer�� ��� ����ī��Ʈ ���� �� ����
			// ���� ���ӽ� �÷���� ����ī��Ʈ ����
			m_Peer->DecreaseRefCount(CRefCountable::REFCOUNT_INIT);

			// ���� ���� �ݹ� ȣ��
			m_Peer->OnDisconnected();

			// TRACE(_T("disconnect reason : %a"), dr);

			if (g_AsyncEventSinkRecycler)
				g_AsyncEventSinkRecycler->Add(m_Peer);
		}

		// ���⼭ ���� ť�� Push
		// Pool���� Pop �Ҷ� ���������� ���� ���¸� üũ�ؼ�
		// Pending�� ���� ���ӵǸ� ForceClose ��Ų��.
		if (m_SocketState.exchange(SOCKET_STATE_CLOSING) == SOCKET_STATE_USING)
		{
			m_ForceCloseTick = Clock::GetElaspedMilli(CAsyncTcpSocket::FORCE_CLOSE_TICK);
			g_AsyncTcpSocketPool->Push(this);

			// ������ OnDisconnect���� DecreaseRefcount(Init) ȣ��
			if (CAsyncTcpSocket::DisConnectEx(m_Socket, reinterpret_cast<LPOVERLAPPED>(evt->GetTag()), TF_REUSE_SOCKET, 0))
			{
				// true�� ��ȯ�ϸ� �ٷ� �����ߴٴ� ��
				// �� ��쿡�� Iocp���� �Ϸ� �ݹ��� �Ѿ���� �ʴ´�.
				g_AsyncDispatcher->Enqueue(this, evt);
			}
			else
			{
				if (WSAGetLastError() != WSA_IO_PENDING)
				{
					TRACE(_T("post disconnect error : %a"), WSAGetLastError());

					if (evt->DecreaseRefCount() == 0)
					{
						CAsyncTcpEvent::Dealloc(evt);
					}

					DecreaseRefCount(CRefCountable::REFCOUNT_INIT);

					// ���� ���� �����Ű�� Free���·� �ٲ�
					// ������ ����Ǯ�� Push�����Ƿ� ���⼱ ��������
					ForceClose();
					m_SocketState = SOCKET_STATE_FREE;
				}
			}
		}
		else
		{
			assert(false && "socket state invalid");
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief 
/// \param bool success 
/// \param DWORD transferred 
/// \param CAsyncTcpEvent * evt 
/// \return void
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAsyncTcpSocket::OnSendEvent(bool success, DWORD transferred, CAsyncTcpEvent* evt)
{
	DecreaseRefCount(CRefCountable::REFCOUNT_SEND);

	if (m_Peer)
	{
		m_Peer->OnSendEvent(success, transferred, evt);
	}

	if (!success)
	{
		TRACE(_T("send failed"));
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief 
/// \param bool success 
/// \param DWORD transferred 
/// \param CAsyncTcpEvent * evt 
/// \return void
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAsyncTcpSocket::OnRecvEvent(bool success, DWORD transferred, CAsyncTcpEvent* evt)
{
	SafeGuard();

	DecreaseRefCount(CRefCountable::REFCOUNT_RECV);

	if (m_Peer)
	{
		m_Peer->OnRecvEvent(success, transferred, evt);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief 
/// \param bool success 
/// \param CAsyncTcpEvent * evt 
/// \return void
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAsyncTcpSocket::OnConnectEvent(bool success, CAsyncTcpEvent* evt)
{
	SafeGuard();

	bool result = false;

	do
	{
		if (!success)
			break;

		if (m_Connect.exchange(true) != false)
		{
			TRACE(_T("already connected"));
			break;
		}

		if (m_SocketState.exchange(SOCKET_STATE_USING) != SOCKET_STATE_FREE)
		{
			TRACE(_T("socket state invalid"));
			break;
		}
			
		if (!SetUpdateContext(SO_UPDATE_CONNECT_CONTEXT, NULL))
			break;

		if (!SetDefaultption())
			break;

		result = true;

		m_Peer->IncreaseRefCount(CRefCountable::REFCOUNT_INIT);

		// ���� �ݹ� ����
		m_Peer->OnConnected();

		// ������ recvȣ��
		m_Peer->PostRecv();

	} while (0);

	if (!result)
	{
		// ���� �������Ƿ� �������� �̺�Ʈ
		m_Peer->DecreaseRefCount(CRefCountable::REFCOUNT_CONNECT);
		m_Peer->OnDisconnected();

		// ���� ����ī��Ʈ ����
		DecreaseRefCount(CRefCountable::REFCOUNT_INIT);

		ForcePush();

		TRACE(_T("connect failed : %a"), WSAGetLastError());
	}

	// �̺�Ʈ�� ����
	if (evt->DecreaseRefCount() == 0)
	{
		CAsyncTcpEvent::Dealloc(evt);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief 
/// \param bool success 
/// \param CAsyncTcpEvent * evt 
/// \return void
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAsyncTcpSocket::OnAcceptEvent(bool success, CAsyncTcpEvent* evt)
{
	SafeGuard();

	bool result = false;

	// ���� ����
	if (m_Connect.exchange(true) != false)
	{
		assert(false && "already connected");
	}

	// ���� ���� ����
	if (m_SocketState.exchange(SOCKET_STATE_USING) != SOCKET_STATE_FREE)
	{
		assert(false && "invalid socket state");
	}

	do
	{
		if (!success)
			break;

		// �Ǿ� �ϳ��� ������
		CAsyncTcpPeer* peer = m_Listener->PopPeer();

		// ���ϰ� ����
		SetPeer(peer);
		peer->IncreaseRefCount(CRefCountable::REFCOUNT_INIT);

		if (!SetUpdateContext(SO_UPDATE_ACCEPT_CONTEXT, m_Listener->GetListenSock()))
			break;

		if (!SetDefaultption())
			break;

		// �ּ� ������
		SOCKADDR_IN* localAddr = nullptr;
		SOCKADDR_IN* remoteAddr = nullptr;
		int locallen = 0;
		int remotelen = 0;
		CAsyncTcpSocket::GetAcceptExSockaddrs(
			evt->m_Buf,
			0,
			sizeof(SOCKADDR_IN) + 16,
			sizeof(SOCKADDR_IN) + 16,
			(sockaddr**)&localAddr, &locallen,
			(sockaddr**)&remoteAddr, &remotelen
		);

		// �ּ� ����
		SetAddr(*remoteAddr);

		// ������� ���� ����
		result = true;

		// ���� �ݹ� ����
		m_Peer->OnConnected();

		// ���� ���ú�
		m_Peer->PostRecv();

	} while (0);

	if (!result)
	{
		int error = WSAGetLastError();
		
		// Accept���� PostDisconnect
		if (error == ERROR_NETNAME_DELETED)
		{
			PostDisconnect(CAsyncTcpEvent::DR_ACCEPT_FAIL);
		}
		// ������ ���� �����
		else if (error == WSA_OPERATION_ABORTED)
		{
			// ���� ����
			DecreaseRefCount(CRefCountable::REFCOUNT_INIT);
			ForcePush();
		}
	}

	// �̺�Ʈ ����ī��Ʈ ���̰� ���� Accept�̺�Ʈ Post�Ҷ� ���
	evt->DecreaseRefCount();

	// ������ �������̸� ���ο� AcceptEvent Post (�ִ� Acceptor�� �����ϱ� ���� �̺�Ʈ)
	if (IsAppRunning())
	{
		// ���ο� AcceptEvent Post
		CAsyncTcpSocket* newAcceptSocket = g_AsyncTcpSocketPool->Pop();
		newAcceptSocket->PostAccept(m_Listener, evt);
	}
	else
	{
		CAsyncTcpEvent::Dealloc(evt);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief 
/// \param bool success 
/// \param CAsyncTcpEvent * evt 
/// \param CAsyncTcpEvent::DisconnectReason dr 
/// \return void
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAsyncTcpSocket::OnDisconnectEvent(bool success, CAsyncTcpEvent* evt, CAsyncTcpEvent::DisconnectReason dr)
{
	SafeGuard();

	// ���� �÷��� ����ī��Ʈ ����
	DecreaseRefCount(CAsyncEventSink::REFCOUNT_INIT);

	if (success)
	{
		assert(m_SocketState == SOCKET_STATE_CLOSING && "socket state must closing");
	}
	else
	{
		assert(m_SocketState == SOCKET_STATE_FORCECLOSED && "socket state must force closed");

		TRACE(_T("socket force closed"));
	}
	
	// ���� ���� ��Ȱ�� ���ɻ��·� ����
	m_ForceCloseTick = Clock::BOUNDLESS;
	m_SocketState = SOCKET_STATE_FREE;

	if (evt->DecreaseRefCount() == 0)
	{
		CAsyncTcpEvent::Dealloc(evt);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief ������ �⺻ �ɼ��� �����Ѵ�.
/// \return bool ��������
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CAsyncTcpSocket::SetDefaultption()
{
	SafeGuard();

	bool result = false;

	do
	{
// 		if (!SetLingerTime(0))
// 			break;

		if (!DisableNagle(true))
			break;

		if (!SetRecvBuffer(0))
			break;

		if (!SetSendBuffer(0))
			break;

		// ����������� ����
		result = true;

	} while (0);

	return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief 
/// \param bool value 
/// \param int time 
/// \return void
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CAsyncTcpSocket::SetLingerTime(unsigned short time)
{
	SafeGuard();

	bool result = true;
	
	// No Time_wait
	linger ling;
	ling.l_onoff = 1;
	ling.l_linger = time;

	if (setsockopt(m_Socket, SOL_SOCKET, SO_LINGER, (const char*)(&ling), sizeof(linger)) != 0)
	{
		TRACE(_T("setlinger error %a"), GetLastError());

		result = false;
	}

	return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief 
/// \param bool value 
/// \return void
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CAsyncTcpSocket::DisableNagle(bool value)
{
	SafeGuard();

	bool result = true;
	// No NagleAlgorithm
	BOOL flag = value ? 1 : 0;
	if (setsockopt(m_Socket, IPPROTO_TCP, TCP_NODELAY, (const char*)&flag, sizeof(flag)) != 0)
	{
		TRACE(_T("disable nalgorithm error : %a"), GetLastError());

		result = false;
	}

	return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief 
/// \param bool value 
/// \return bool
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CAsyncTcpSocket::SetReuseAddr(bool value)
{
	SafeGuard();

	bool result = true;
	// ReuseAddr
	int opt = value ? 1 : 0;
	if (setsockopt(m_Socket, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(int)) != 0)
	{
		TRACE(_T("socket reuseAddr error : %a"), GetLastError());

		result = false;
	}

	return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief 
/// \param int size 
/// \return bool
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CAsyncTcpSocket::SetRecvBuffer(int size)
{
	SafeGuard();

	bool result = true;
	if (setsockopt(m_Socket, SOL_SOCKET, SO_RCVBUF, (const char*)&size, sizeof(size)) != 0)
	{
		TRACE(_T("socket set recv buffer error : %a"), GetLastError());

		result = false;
	}

	return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief 
/// \param int size 
/// \return bool
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CAsyncTcpSocket::SetSendBuffer(int size)
{
	SafeGuard();

	bool result = true;
	if (setsockopt(m_Socket, SOL_SOCKET, SO_SNDBUF, (const char*)&size, sizeof(size)) != 0)
	{
		TRACE(_T("socket set send buffer error : %a"), GetLastError());

		result = false;
	}

	return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief 
/// \param int option 
/// \param SOCKET sock 
/// \return bool
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CAsyncTcpSocket::SetUpdateContext(int option, SOCKET inheritedSock)
{
	SafeGuard();

	bool result = true;

	int oplen = inheritedSock != NULL ? sizeof(inheritedSock) : 0;
	const char* opval = inheritedSock != NULL ? (const char*)&inheritedSock : NULL;
	
	if (setsockopt(m_Socket, SOL_SOCKET, option, opval, oplen) != 0)
	{
		TRACE(_T("socket update context error : %a"), GetLastError());

		result = false;
	}

	return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief �̺�Ʈ ���� �ݹ�
/// \return void
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAsyncTcpSocket::ForcePush()
{
	SafeGuard();

	// ���� �����Ű��
	ForceClose();

	// ���� ���� ���·� ���� ��
	m_SocketState = SOCKET_STATE_FREE;

	// ���� ť�� Push
	g_AsyncTcpSocketPool->Push(this);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Iocp�� ���ϰ� �Ǿ ���� ��Ų��.
/// \return bool true�� ����
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CAsyncTcpSocket::AssociateIocp()
{
	bool success = true;

	if (!g_AsyncDispatcher->Associate(reinterpret_cast<HANDLE>(m_Socket), reinterpret_cast<ULONG_PTR>(this)))
	{
		TRACE(_T("associate error"));

		success = false;
	}

	return success;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief 
/// \return bool
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CAsyncTcpSocket::Setup()
{
    SafeGuard();

	bool result = false;

	WSAData wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) == 0)
	{
		result = true;

		LoadExtensionFn();
	}

	return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief 
/// \return void
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAsyncTcpSocket::Teardown()
{
	SafeGuard();

	WSACleanup();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief AcceptEx �Լ�
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CAsyncTcpSocket::AcceptEx(
	SOCKET sListenSocket,
	SOCKET sAcceptSocket,
	PVOID lpOutputBuffer,
	DWORD dwReceiveDataLength,
	DWORD dwLocalAddressLength,
	DWORD dwRemoteAddressLength,
	LPDWORD lpdwBytesReceived,
	LPOVERLAPPED lpOverlapped)
{
	return g_FnAcceptEx(
		sListenSocket,
		sAcceptSocket,
		lpOutputBuffer,
		dwReceiveDataLength,
		dwLocalAddressLength,
		dwRemoteAddressLength,
		lpdwBytesReceived,
		lpOverlapped);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief ConnectEx �Լ�
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CAsyncTcpSocket::ConnectEx(SOCKET socket,
	const sockaddr* addr,
	int namelen,
	PVOID lpsendbuffer,
	DWORD senddatalength,
	LPDWORD bytessent,
	LPOVERLAPPED overlapped)
{
	return g_FnConnectEx(
		socket,
		addr,
		namelen,
		lpsendbuffer,
		senddatalength,
		bytessent,
		overlapped
		);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief DisConnectEx �Լ�
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CAsyncTcpSocket::DisConnectEx(
	SOCKET socket,
	LPOVERLAPPED overlapped,
	DWORD flag,
	DWORD reserved)
{
	return g_FnDisconnectEx(socket, overlapped, flag, reserved);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief GetAcceptExSockaddrs �Լ�
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAsyncTcpSocket::GetAcceptExSockaddrs(
	PVOID lpOutputBuffer,
	DWORD dwReceiveDataLength,
	DWORD dwLocalAddressLength,
	DWORD dwRemoteAddressLength,
	sockaddr **LocalSockaddr,
	LPINT LocalSockaddrLength,
	sockaddr **RemoteSockaddr,
	LPINT RemoteSockaddrLength)
{
	g_FnGetAcceptExSockAddrs(
		lpOutputBuffer,
		dwReceiveDataLength,
		dwLocalAddressLength,
		dwRemoteAddressLength,
		LocalSockaddr,
		LocalSockaddrLength,
		RemoteSockaddr,
		RemoteSockaddrLength
		);
}
