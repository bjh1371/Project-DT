////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file AsyncTcpEventSink.h
/// \author bjh1371
/// \date 2015/07/02
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "AsyncEventSink.h"

#include "AsyncTcpEvent.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \class CAsyncTcpEventSink
/// \brief 
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CAsyncTcpEventSink : public CAsyncEventSink
{
private:


public:
	/// \brief 생성자
	CAsyncTcpEventSink();
	
	/// \brief 소멸자
	virtual ~CAsyncTcpEventSink();


public:
	/// \brief Receive 이벤트 종류시 호출
	virtual void OnRecvEvent(bool success, DWORD transferred, CAsyncTcpEvent* evt);

	/// \brief Send 이벤트 종료시 호출
	virtual void OnSendEvent(bool success, DWORD transferred, CAsyncTcpEvent* evt);

	/// \brief Send 이벤트 종료시 호출
	virtual void OnDisconnectEvent(bool success, CAsyncTcpEvent* evt, CAsyncTcpEvent::DisconnectReason dr);

	/// \brief Accept 이벤트 종료시 호출
	virtual void OnAcceptEvent(bool success, CAsyncTcpEvent* evt);

	/// \brief Connect 이벤트 종료시 호출
	virtual void OnConnectEvent(bool success, CAsyncTcpEvent* evt);
};