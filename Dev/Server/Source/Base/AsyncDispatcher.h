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
/// \brief IOCP 처리자
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CAsyncDispatcher
{
private:
	class CWorkerThread;

	typedef stx::vector<CWorkerThread*> CWorkerThreadArray;
	typedef OVERLAPPED CShutDownEvent;

private:
	HANDLE             m_Iocp;              ///< IOCP 핸들
	CWorkerThreadArray m_WorkerThreadArray; ///< 워커스레드 배열
	CShutDownEvent     m_ShutdownEvent;     ///< 셧다운 이벤트
	

public:
	/// \brief 생성자
	CAsyncDispatcher();

	/// \brief 소멸자
	~CAsyncDispatcher();


public:
	/// \brief IOCP에 연결한다.
	bool Associate(HANDLE handle, ULONG_PTR key);

	/// \brief IOCP에 이벤트를 집어넣는다.
	bool Enqueue(CAsyncEventSink* key, CAsyncEvent* evt);

	/// \brief IOCP에 이벤트를 집어넣는다.
	bool Enqueue(CAsyncTcpSocket* socket, CAsyncEvent* evt);


public:
	HANDLE GetIocp() const { return m_Iocp; }
	void SetIocp(HANDLE val) { m_Iocp = val; }


public:
	/// \brief 워커스레드 시작
	void Start(unsigned int workerThreadCount = 0);

	/// \brief 워커스레드 정지
	void Stop();


private:
	/// \brief 워커 스레드
	void WorkerThread();
};

extern CAsyncDispatcher* g_AsyncDispatcher;