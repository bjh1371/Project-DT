////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file AsyncTcpEvent.h
/// \author bjh1371
/// \date 2015/07/02
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "AsyncEvent.h"
#include "RefCountable.h"

class CAsyncTcpSocket;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \class CAsyncTcpEvent
/// \brief Tcp�� ����� �̺�Ʈ
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CAsyncTcpEvent : public CAsyncEvent
{
public:
	enum OP_TYPE
	{
		OP_RECV,
		OP_SEND,
		OP_CONNECT,
		OP_ACCEPT,
		OP_DISCONNECT,
		OP_MAX,
	};

	enum DisconnectReason
	{
		DR_SEND_FAIL,
		DR_RECV_FAIL,
		DR_ACCEPT_FAIL,
		DR_CONNECT_FAIL,
		DR_DISCONNECT_FAIL,
		DR_FORCE,
		DR_MAX,
	};

	// ���� �����
	enum
	{
		ACCEPT_BUF_SIZE = 64, ///< AcceptEvent�� ũ��

		MAX_CAPACITY = 65535, ///< ���� ũ��
	};


public:
	int               m_Capacity;     ///< �̺�Ʈ ũ��
	int               m_TotalBytes;   ///< ���� ������� �� ũ��
	OP_TYPE           m_OpType;       ///< ���� �̺�Ʈ ����
	char*             m_Buf;          ///< ���� ����
	WSABUF            m_WsaBuf;       ///< WSA ����
	DisconnectReason  m_DisconReason; ///< ���� ���� ����
	CAsyncTcpSocket*  m_Owner;        ///< Accept �� �� �̺�Ʈ�� ���� ��ũ


public:
	/// \brief ������
	CAsyncTcpEvent(int capacity);
	
	/// \brief �Ҹ���
	virtual ~CAsyncTcpEvent();
	
	
public:
	/// \brief ����
	virtual void Execute(bool success, DWORD transferred, ULONG_PTR target);


public:
	/// \brief �̺�Ʈ�� ���� ��Ų��.
	void Reset();


public:
	/// \brief �޸� �Ҵ�
	static CAsyncTcpEvent* Alloc(int capacity);

	/// \brief �޸� ����
	static void Dealloc(CAsyncTcpEvent* evt);


public:
	/// \brief �����ִ� Event ����
	static int GetCurrentEvent();
};