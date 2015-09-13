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
/// \brief Tcp시 사용할 이벤트
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

	// 각종 상수들
	enum
	{
		ACCEPT_BUF_SIZE = 64, ///< AcceptEvent시 크기

		MAX_CAPACITY = 65535, ///< 버퍼 크기
	};


public:
	int               m_Capacity;     ///< 이벤트 크기
	int               m_TotalBytes;   ///< 현재 사용중인 총 크기
	OP_TYPE           m_OpType;       ///< 현재 이벤트 종류
	char*             m_Buf;          ///< 실제 버퍼
	WSABUF            m_WsaBuf;       ///< WSA 버퍼
	DisconnectReason  m_DisconReason; ///< 접속 종료 이유
	CAsyncTcpSocket*  m_Owner;        ///< Accept 시 이 이벤트를 던진 싱크


public:
	/// \brief 생성자
	CAsyncTcpEvent(int capacity);
	
	/// \brief 소멸자
	virtual ~CAsyncTcpEvent();
	
	
public:
	/// \brief 실행
	virtual void Execute(bool success, DWORD transferred, ULONG_PTR target);


public:
	/// \brief 이벤트를 리셋 시킨다.
	void Reset();


public:
	/// \brief 메모리 할당
	static CAsyncTcpEvent* Alloc(int capacity);

	/// \brief 메모리 해제
	static void Dealloc(CAsyncTcpEvent* evt);


public:
	/// \brief 남아있는 Event 숫자
	static int GetCurrentEvent();
};