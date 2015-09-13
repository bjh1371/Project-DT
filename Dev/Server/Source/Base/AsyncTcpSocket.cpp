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

	// 확장 함수 로드
	void LoadExtensionFn()
	{
		SafeGuard();

		// 확장 함수 로드용 소켓
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

		// 확장 함수 로드용 소켓 닫기
		if (loadFnSocket != INVALID_SOCKET)
		{
			closesocket(loadFnSocket);
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief 생성자
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
		// IOCP와 연결
		AssociateIocp();
	}
	else
	{
		TRACE(_T("WSASocket Error"));
	}

	memset(&m_Addr, 0, sizeof(m_Addr));
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief 소멸자
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
/// \brief 피어와 소켓을 연결한다.
/// \param CAsyncTcpPeer * peer 연결할 피어
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
/// \brief 소켓 강제로 종료
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAsyncTcpSocket::ForceClose()
{
	SafeGuard();

	if (m_Socket != INVALID_SOCKET)
	{
		// 강제 종료 상태로 바꾸고
		m_SocketState = SOCKET_STATE_FORCECLOSED;
		
		// 강제 종료
		if (closesocket(m_Socket) == 0)
		{
			m_Socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
			m_Peer = nullptr;
			m_Bind = false;
			m_Connect = false;

			// 소켓을 새로 생성했으므로 Iocp에 연결
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

	// 접속했고 Disconnect 이벤트를 포스트하지 않았으면 Send한다.
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
	// Peer에서 올리고 들어온 레퍼카운트 감소
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
	// Peer에서 올리고 들어온 레퍼카운트 감소
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
	
	// 이벤트가 있으면 바로 사용
	evt = evt ? evt : CAsyncTcpEvent::Alloc(CAsyncTcpEvent::ACCEPT_BUF_SIZE);

	evt->m_OpType = CAsyncTcpEvent::OP_ACCEPT;
	evt->m_Owner = this;

	// 최초 레퍼카운트 증가
	IncreaseRefCount(CAsyncEventSink::REFCOUNT_INIT);
	evt->IncreaseRefCount();

	DWORD bytesReceived = 0;
	if (CAsyncTcpSocket::AcceptEx(
		m_Listener->GetListenSock(),
		m_Socket, evt->m_Buf, 0,
		sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16,
		&bytesReceived, reinterpret_cast<LPOVERLAPPED>(evt->GetTag())))
	{
		// true를 반환하면 바로 성공했다는 뜻
		// 이 경우에는 Iocp에서 완료 콜백이 넘어오지 않는다.
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

	// 최초 레퍼카운트 증가
	IncreaseRefCount(CAsyncEventSink::REFCOUNT_INIT);
	evt->IncreaseRefCount();

	// family를 제외하고 0으로 초기화 해서 bind한다.
	m_Addr.sin_family = AF_INET;
	m_Addr.sin_port = htons(0);
	m_Addr.sin_addr.s_addr = htonl(INADDR_ANY);


	if (!m_Bind && bind(m_Socket, (SOCKADDR*)&m_Addr, sizeof(m_Addr)) != 0)
	{
		TRACE(_T("bind error : %a"), WSAGetLastError());
	}
	// Bind 성공
	else
	{
		m_Bind = true;
	}

	InetPton(AF_INET, address, &m_Addr.sin_addr.s_addr);
	m_Addr.sin_port = htons(port);

	DWORD bytessent = 0;
	if (CAsyncTcpSocket::ConnectEx(m_Socket, (SOCKADDR*)&m_Addr, sizeof(m_Addr), NULL, 0, &bytessent, reinterpret_cast<LPOVERLAPPED>(evt->GetTag())))
	{
		// true를 반환하면 바로 성공했다는 뜻
		// 이 경우에는 Iocp에서 완료 콜백이 넘어오지 않는다.
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
			// 이 단계에서 Peer는 모든 레퍼카운트 감소 후 삭제
			// 최초 접속시 올려줬던 레퍼카운트 감소
			m_Peer->DecreaseRefCount(CRefCountable::REFCOUNT_INIT);

			// 접속 종료 콜백 호출
			m_Peer->OnDisconnected();

			// TRACE(_T("disconnect reason : %a"), dr);

			if (g_AsyncEventSinkRecycler)
				g_AsyncEventSinkRecycler->Add(m_Peer);
		}

		// 여기서 소켓 큐에 Push
		// Pool에서 Pop 할때 지속적으로 소켓 상태를 체크해서
		// Pending이 오래 지속되면 ForceClose 시킨다.
		if (m_SocketState.exchange(SOCKET_STATE_CLOSING) == SOCKET_STATE_USING)
		{
			m_ForceCloseTick = Clock::GetElaspedMilli(CAsyncTcpSocket::FORCE_CLOSE_TICK);
			g_AsyncTcpSocketPool->Push(this);

			// 소켓은 OnDisconnect에서 DecreaseRefcount(Init) 호출
			if (CAsyncTcpSocket::DisConnectEx(m_Socket, reinterpret_cast<LPOVERLAPPED>(evt->GetTag()), TF_REUSE_SOCKET, 0))
			{
				// true를 반환하면 바로 성공했다는 뜻
				// 이 경우에는 Iocp에서 완료 콜백이 넘어오지 않는다.
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

					// 소켓 강제 종료시키고 Free상태로 바꿈
					// 위에서 소켓풀에 Push했으므로 여기선 하지않음
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

		// 접속 콜백 실행
		m_Peer->OnConnected();

		// 최초의 recv호출
		m_Peer->PostRecv();

	} while (0);

	if (!result)
	{
		// 접속 못했으므로 접속종료 이벤트
		m_Peer->DecreaseRefCount(CRefCountable::REFCOUNT_CONNECT);
		m_Peer->OnDisconnected();

		// 소켓 레퍼카운트 감소
		DecreaseRefCount(CRefCountable::REFCOUNT_INIT);

		ForcePush();

		TRACE(_T("connect failed : %a"), WSAGetLastError());
	}

	// 이벤트는 삭제
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

	// 접속 설정
	if (m_Connect.exchange(true) != false)
	{
		assert(false && "already connected");
	}

	// 소켓 상태 변경
	if (m_SocketState.exchange(SOCKET_STATE_USING) != SOCKET_STATE_FREE)
	{
		assert(false && "invalid socket state");
	}

	do
	{
		if (!success)
			break;

		// 피어 하나를 빼오고
		CAsyncTcpPeer* peer = m_Listener->PopPeer();

		// 소켓과 연결
		SetPeer(peer);
		peer->IncreaseRefCount(CRefCountable::REFCOUNT_INIT);

		if (!SetUpdateContext(SO_UPDATE_ACCEPT_CONTEXT, m_Listener->GetListenSock()))
			break;

		if (!SetDefaultption())
			break;

		// 주소 얻어오고
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

		// 주소 저장
		SetAddr(*remoteAddr);

		// 여기까지 오면 성공
		result = true;

		// 접속 콜백 실행
		m_Peer->OnConnected();

		// 최초 리시브
		m_Peer->PostRecv();

	} while (0);

	if (!result)
	{
		int error = WSAGetLastError();
		
		// Accept실패 PostDisconnect
		if (error == ERROR_NETNAME_DELETED)
		{
			PostDisconnect(CAsyncTcpEvent::DR_ACCEPT_FAIL);
		}
		// 강제로 서버 종료시
		else if (error == WSA_OPERATION_ABORTED)
		{
			// 소켓 정리
			DecreaseRefCount(CRefCountable::REFCOUNT_INIT);
			ForcePush();
		}
	}

	// 이벤트 레퍼카운트 줄이고 다음 Accept이벤트 Post할때 사용
	evt->DecreaseRefCount();

	// 서버가 실행중이면 새로운 AcceptEvent Post (최대 Acceptor를 유지하기 위한 이벤트)
	if (IsAppRunning())
	{
		// 새로운 AcceptEvent Post
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

	// 최초 올려준 레퍼카운트 감소
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
	
	// 소켓 상태 재활용 가능상태로 변경
	m_ForceCloseTick = Clock::BOUNDLESS;
	m_SocketState = SOCKET_STATE_FREE;

	if (evt->DecreaseRefCount() == 0)
	{
		CAsyncTcpEvent::Dealloc(evt);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief 소켓의 기본 옵션을 적용한다.
/// \return bool 성공여부
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

		// 여기까지오면 성공
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
/// \brief 이벤트 실패 콜백
/// \return void
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAsyncTcpSocket::ForcePush()
{
	SafeGuard();

	// 강제 종료시키고
	ForceClose();

	// 소켓 재사용 상태로 셋팅 후
	m_SocketState = SOCKET_STATE_FREE;

	// 소켓 큐에 Push
	g_AsyncTcpSocketPool->Push(this);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Iocp에 소켓과 피어를 연관 시킨다.
/// \return bool true면 성공
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
/// \brief AcceptEx 함수
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
/// \brief ConnectEx 함수
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
/// \brief DisConnectEx 함수
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
/// \brief GetAcceptExSockaddrs 함수
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
