////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file AsyncTcpSocket.h
/// \author bjh1371
/// \date 2015/07/08
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "AsyncTcpEvent.h"
#include "RefCountable.h"

#include <MSWSock.h>

class CAsyncTcpPeer;
class CAsyncPeerListener;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \class CAsyncTcpSocket
/// \brief Tcp 통신을하는 소켓
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CAsyncTcpSocket : public CRefCountable
{
private:
	enum
	{
		FORCE_CLOSE_TICK = 1000 * 60, ///< 강제로 종료할 틱 1분
	};


	enum SocketState
	{
		SOCKET_STATE_FREE,        ///< 사용 가능
		SOCKET_STATE_USING,       ///< 사용중
		SOCKET_STATE_CLOSING,     ///< 종료하는 중
		SOCKET_STATE_FORCECLOSED, ///< 강제 종료
	};


private:
	SOCKET                   m_Socket;         ///< 소켓
	SOCKADDR_IN              m_Addr;           ///< 주소
	CAsyncTcpPeer*           m_Peer;           ///< 이 소켓과 연결된 피어객체 (IOCP 에도 이 두 객체가 연결된다)
	CAsyncPeerListener*      m_Listener;       ///< 이 소켓과 연결된 리스너
	std::atomic<SocketState> m_SocketState;    ///< 소켓 상태
	Milli_t                  m_ForceCloseTick; ///< 강제로 소켓을 닫을 틱
	std::atomic<bool>        m_Connect;        ///< 접속여부
	bool                     m_Bind;           ///< 이미 Bind되어 있는가? (소켓 재사용을 위해 한번만 바인드)


public:
	/// \brief 생성자
	CAsyncTcpSocket();
	
	/// \brief 소멸자
	~CAsyncTcpSocket();


public:
	/// \brief 피어를 셋팅한다.
	void SetPeer(CAsyncTcpPeer* peer);


public:
	/// \brief 접속 되있는 상태인가?
	bool IsConnected() const { return m_Connect; }

	/// \brief 긴 시간 펜딩된 소켓인가?
	bool IsLongPendingSocket() const { return m_SocketState == SOCKET_STATE_CLOSING && Clock::IsPast(m_ForceCloseTick); }

	/// \brief 사용 가능 소켓인가?
	bool IsFreeSocket() const { return m_SocketState == SOCKET_STATE_FREE && IsSafeToDelete(); }

	/// \brief 소켓 강제로 종료
	void ForceClose();


public:
	/// \brief Send 이벤트 등록
	void PostSend(CAsyncTcpEvent* evt, int totalBytes);

	/// \brief Recv 이벤트 등록
	void PostRecv(CAsyncTcpEvent* evt, int offset = 0);

	/// \brief Connect 이벤트 등록
	void PostConnect(LPCTSTR address, short port);

	/// \brief AcceptEvent 이벤트 등록
	bool PostAccept(CAsyncPeerListener* listener, CAsyncTcpEvent* evt = nullptr);

	/// \brief Disconnect 이벤트 등록
	void PostDisconnect(CAsyncTcpEvent::DisconnectReason dr);


public:
	/// \brief Send 완료 콜백
	void OnSendEvent(bool success, DWORD transferred, CAsyncTcpEvent* evt);

	/// \brief Recv 완료 콜백
	void OnRecvEvent(bool success, DWORD transferred, CAsyncTcpEvent* evt);

	/// \brief Accept 완료 콜백
	void OnAcceptEvent(bool success, CAsyncTcpEvent* evt);

	/// \brief Connect 완료 콜백
	void OnConnectEvent(bool success, CAsyncTcpEvent* evt);

	/// \brief Disconnect 완료 콜백
	void OnDisconnectEvent(bool success, CAsyncTcpEvent* evt, CAsyncTcpEvent::DisconnectReason dr);


public:
	/// \brief 소켓 기본 옵션을 셋팅한다.
	bool SetDefaultption();

	/// \brief 링거 타임 셋팅
	bool SetLingerTime(unsigned short time);

	/// \brief Disable Nagle
	bool DisableNagle(bool value);

	/// \brief 주소 재사용 셋팅
	bool SetReuseAddr(bool value);

	/// \brief Recv 버퍼 사이즈
	bool SetRecvBuffer(int size);

	/// \brief Send 버퍼 사이즈
	bool SetSendBuffer(int size);
	
	/// \brief 소켓 내용 업데이트
	bool SetUpdateContext(int option, SOCKET inheritedSock);


private:
	/// \brief 강제로 큐에 Push 
	void ForcePush();

	/// \brief Iocp에 소켓과 피어를 연결한다.
	bool AssociateIocp();


public:
	SOCKET GetSocket() const { return m_Socket; }
	void SetSocket(SOCKET val) { m_Socket = val; }
	
	SOCKADDR_IN GetAddr() const { return m_Addr; }
	void SetAddr(SOCKADDR_IN val) { m_Addr = val; }

	SocketState GetSocketState() const { return m_SocketState.load(); }

	Milli_t GetClosedTick() const { return m_ForceCloseTick; }
	void SetClosedTick(Milli_t val) { m_ForceCloseTick = val; }


public:
	/// \brief 셋업
	static bool Setup();

	/// \brief 자원해제
	static void Teardown();


public:
	/// \brief AcceptEx 함수
	static BOOL AcceptEx(
		SOCKET sListenSocket,
		SOCKET sAcceptSocket,
		PVOID lpOutputBuffer,
		DWORD dwReceiveDataLength,
		DWORD dwLocalAddressLength,
		DWORD dwRemoteAddressLength,
		LPDWORD lpdwBytesReceived,
		LPOVERLAPPED lpOverlapped
		);

	/// \brief connectEx 함수
	static BOOL ConnectEx(
		SOCKET socket,
		const sockaddr* addr,
		int namelen,
		PVOID lpsendbuffer,
		DWORD senddatalength,
		LPDWORD bytessent,
		LPOVERLAPPED overlapped
		);

	/// \brief DisconnectEx 함수
	static BOOL DisConnectEx(
		SOCKET socket,
		LPOVERLAPPED overlapped,
		DWORD flag,
		DWORD reserved
		);

	/// \brief GetAcceptExSockaddrs 함수
	static void GetAcceptExSockaddrs(
		PVOID lpOutputBuffer,
		DWORD dwReceiveDataLength,
		DWORD dwLocalAddressLength,
		DWORD dwRemoteAddressLength,
		sockaddr **LocalSockaddr,
		LPINT LocalSockaddrLength,
		sockaddr **RemoteSockaddr,
		LPINT RemoteSockaddrLength
		);
};