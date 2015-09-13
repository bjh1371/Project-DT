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
	/// \brief ������
	CAsyncTcpEventSink();
	
	/// \brief �Ҹ���
	virtual ~CAsyncTcpEventSink();


public:
	/// \brief Receive �̺�Ʈ ������ ȣ��
	virtual void OnRecvEvent(bool success, DWORD transferred, CAsyncTcpEvent* evt);

	/// \brief Send �̺�Ʈ ����� ȣ��
	virtual void OnSendEvent(bool success, DWORD transferred, CAsyncTcpEvent* evt);

	/// \brief Send �̺�Ʈ ����� ȣ��
	virtual void OnDisconnectEvent(bool success, CAsyncTcpEvent* evt, CAsyncTcpEvent::DisconnectReason dr);

	/// \brief Accept �̺�Ʈ ����� ȣ��
	virtual void OnAcceptEvent(bool success, CAsyncTcpEvent* evt);

	/// \brief Connect �̺�Ʈ ����� ȣ��
	virtual void OnConnectEvent(bool success, CAsyncTcpEvent* evt);
};