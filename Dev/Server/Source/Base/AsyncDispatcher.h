////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file CAsyncDispatcher.h
/// \author 
/// \date 2014.10.24
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "AsyncEvent.h"

class CAsyncEventSink;
class CAsyncTcpSocket;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \class CAsyncDispatcher
/// \brief IOCP ó����
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CAsyncDispatcher
{
private:
	class CWorkerThread;

	typedef stx::vector<CWorkerThread*> CWorkerThreadArray;
	typedef OVERLAPPED CShutDownEvent;

private:
	HANDLE             m_Iocp;              ///< IOCP �ڵ�
	CWorkerThreadArray m_WorkerThreadArray; ///< ��Ŀ������ �迭
	CShutDownEvent     m_ShutdownEvent;     ///< �˴ٿ� �̺�Ʈ
	

public:
	/// \brief ������
	CAsyncDispatcher();

	/// \brief �Ҹ���
	~CAsyncDispatcher();


public:
	/// \brief IOCP�� �����Ѵ�.
	bool Associate(HANDLE handle, ULONG_PTR key);

	/// \brief IOCP�� �̺�Ʈ�� ����ִ´�.
	bool Enqueue(CAsyncEventSink* key, CAsyncEvent* evt);

	/// \brief IOCP�� �̺�Ʈ�� ����ִ´�.
	bool Enqueue(CAsyncTcpSocket* socket, CAsyncEvent* evt);


public:
	HANDLE GetIocp() const { return m_Iocp; }
	void SetIocp(HANDLE val) { m_Iocp = val; }


public:
	/// \brief ��Ŀ������ ����
	void Start(unsigned int workerThreadCount = 0);

	/// \brief ��Ŀ������ ����
	void Stop();


private:
	/// \brief ��Ŀ ������
	void WorkerThread();
};

extern CAsyncDispatcher* g_AsyncDispatcher;