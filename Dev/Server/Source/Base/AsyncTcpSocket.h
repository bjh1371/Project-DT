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
/// \brief Tcp ������ϴ� ����
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CAsyncTcpSocket : public CRefCountable
{
private:
	enum
	{
		FORCE_CLOSE_TICK = 1000 * 60, ///< ������ ������ ƽ 1��
	};


	enum SocketState
	{
		SOCKET_STATE_FREE,        ///< ��� ����
		SOCKET_STATE_USING,       ///< �����
		SOCKET_STATE_CLOSING,     ///< �����ϴ� ��
		SOCKET_STATE_FORCECLOSED, ///< ���� ����
	};


private:
	SOCKET                   m_Socket;         ///< ����
	SOCKADDR_IN              m_Addr;           ///< �ּ�
	CAsyncTcpPeer*           m_Peer;           ///< �� ���ϰ� ����� �Ǿü (IOCP ���� �� �� ��ü�� ����ȴ�)
	CAsyncPeerListener*      m_Listener;       ///< �� ���ϰ� ����� ������
	std::atomic<SocketState> m_SocketState;    ///< ���� ����
	Milli_t                  m_ForceCloseTick; ///< ������ ������ ���� ƽ
	std::atomic<bool>        m_Connect;        ///< ���ӿ���
	bool                     m_Bind;           ///< �̹� Bind�Ǿ� �ִ°�? (���� ������ ���� �ѹ��� ���ε�)


public:
	/// \brief ������
	CAsyncTcpSocket();
	
	/// \brief �Ҹ���
	~CAsyncTcpSocket();


public:
	/// \brief �Ǿ �����Ѵ�.
	void SetPeer(CAsyncTcpPeer* peer);


public:
	/// \brief ���� ���ִ� �����ΰ�?
	bool IsConnected() const { return m_Connect; }

	/// \brief �� �ð� ����� �����ΰ�?
	bool IsLongPendingSocket() const { return m_SocketState == SOCKET_STATE_CLOSING && Clock::IsPast(m_ForceCloseTick); }

	/// \brief ��� ���� �����ΰ�?
	bool IsFreeSocket() const { return m_SocketState == SOCKET_STATE_FREE && IsSafeToDelete(); }

	/// \brief ���� ������ ����
	void ForceClose();


public:
	/// \brief Send �̺�Ʈ ���
	void PostSend(CAsyncTcpEvent* evt, int totalBytes);

	/// \brief Recv �̺�Ʈ ���
	void PostRecv(CAsyncTcpEvent* evt, int offset = 0);

	/// \brief Connect �̺�Ʈ ���
	void PostConnect(LPCTSTR address, short port);

	/// \brief AcceptEvent �̺�Ʈ ���
	bool PostAccept(CAsyncPeerListener* listener, CAsyncTcpEvent* evt = nullptr);

	/// \brief Disconnect �̺�Ʈ ���
	void PostDisconnect(CAsyncTcpEvent::DisconnectReason dr);


public:
	/// \brief Send �Ϸ� �ݹ�
	void OnSendEvent(bool success, DWORD transferred, CAsyncTcpEvent* evt);

	/// \brief Recv �Ϸ� �ݹ�
	void OnRecvEvent(bool success, DWORD transferred, CAsyncTcpEvent* evt);

	/// \brief Accept �Ϸ� �ݹ�
	void OnAcceptEvent(bool success, CAsyncTcpEvent* evt);

	/// \brief Connect �Ϸ� �ݹ�
	void OnConnectEvent(bool success, CAsyncTcpEvent* evt);

	/// \brief Disconnect �Ϸ� �ݹ�
	void OnDisconnectEvent(bool success, CAsyncTcpEvent* evt, CAsyncTcpEvent::DisconnectReason dr);


public:
	/// \brief ���� �⺻ �ɼ��� �����Ѵ�.
	bool SetDefaultption();

	/// \brief ���� Ÿ�� ����
	bool SetLingerTime(unsigned short time);

	/// \brief Disable Nagle
	bool DisableNagle(bool value);

	/// \brief �ּ� ���� ����
	bool SetReuseAddr(bool value);

	/// \brief Recv ���� ������
	bool SetRecvBuffer(int size);

	/// \brief Send ���� ������
	bool SetSendBuffer(int size);
	
	/// \brief ���� ���� ������Ʈ
	bool SetUpdateContext(int option, SOCKET inheritedSock);


private:
	/// \brief ������ ť�� Push 
	void ForcePush();

	/// \brief Iocp�� ���ϰ� �Ǿ �����Ѵ�.
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
	/// \brief �¾�
	static bool Setup();

	/// \brief �ڿ�����
	static void Teardown();


public:
	/// \brief AcceptEx �Լ�
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

	/// \brief connectEx �Լ�
	static BOOL ConnectEx(
		SOCKET socket,
		const sockaddr* addr,
		int namelen,
		PVOID lpsendbuffer,
		DWORD senddatalength,
		LPDWORD bytessent,
		LPOVERLAPPED overlapped
		);

	/// \brief DisconnectEx �Լ�
	static BOOL DisConnectEx(
		SOCKET socket,
		LPOVERLAPPED overlapped,
		DWORD flag,
		DWORD reserved
		);

	/// \brief GetAcceptExSockaddrs �Լ�
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