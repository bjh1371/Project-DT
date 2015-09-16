////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file CAsyncDispatcher.cpp
/// \author 
/// \date 2014.10.24
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "AsyncDispatcher.h"

#include "AsyncEvent.h"
#include "AsyncEventSink.h"
#include "Thread.h"
#include "ThreadRegistry.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \class CWorkerThread
/// \brief IOCP 워커스레드
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CAsyncDispatcher::CWorkerThread : public CThread
{
private:
	CAsyncDispatcher::CShutDownEvent* m_Shutdown; ///< 셧다운 객체


public:
	/// \brief 생성자
	CWorkerThread(CAsyncDispatcher::CShutDownEvent* shutdown)
	:
	m_Shutdown(shutdown)
	{
		Start();
	}

	virtual ~CWorkerThread() {}


public:
	/// \brief 정지
	virtual void Stop()
	{
		SafeGuard();

		// 현재 이 워커쓰레드가 정지할 것이라는 보장이 없다.
		PostQueuedCompletionStatus(g_AsyncDispatcher->GetIocp(), 0, 0, m_Shutdown);
	}


private:
	/// \brief 실제 스레드 메인
	virtual void ThreadMain()
	{
		SafeGuard();

		while (1)
		{
			LPOVERLAPPED overlapped = nullptr;
			ULONG_PTR key = NULL;
			DWORD transferred = 0;

			bool success = GetQueuedCompletionStatus(
				g_AsyncDispatcher->GetIocp(),
				&transferred,
				&key,
				&overlapped,
				INFINITE
				) != FALSE;

			// 셧다운 객체일경우
			if (overlapped == m_Shutdown)
				break;

			CAsyncEvent::TAG* tag = reinterpret_cast<CAsyncEvent::TAG*>(overlapped);

			CAsyncEvent* evt = tag->Owner;
			if (evt)
			{
				evt->Execute(success, transferred, key);
			}
			else
			{
				TRACE(_T("evt || sink is null"));
				break;
			}
		}
	}
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief 생성자
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CAsyncDispatcher::CAsyncDispatcher()
:
m_Iocp(INVALID_HANDLE_VALUE)
{
	SafeGuard();

	m_Iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (m_Iocp == INVALID_HANDLE_VALUE)
	{
		TRACE(_T("create iocp failed"));
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief 소멸자
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CAsyncDispatcher::~CAsyncDispatcher()
{
	SafeGuard();

	Stop();

	if (m_Iocp != INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_Iocp);
		m_Iocp = INVALID_HANDLE_VALUE;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief IOCP에 연결한다
/// \param handle 핸들
/// \param key 키
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CAsyncDispatcher::Associate(HANDLE handle, ULONG_PTR key)
{
	SafeGuard();

	bool result = false;
	if (m_Iocp != INVALID_HANDLE_VALUE)
	{
		HANDLE iocp = CreateIoCompletionPort(handle, m_Iocp, key, 0);
		if (m_Iocp == iocp)
		{
			result = true;
		}
	}
	return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief 이벤트를 집어넣는다.
/// \param key 키
/// \param evt 이벤트
/// \return bool 성공여부
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CAsyncDispatcher::Enqueue(CAsyncEventSink* key, CAsyncEvent* evt)
{
	SafeGuard();

	return PostQueuedCompletionStatus(m_Iocp, 0, reinterpret_cast<ULONG_PTR>(key), reinterpret_cast<LPOVERLAPPED>(evt->GetTag())) != FALSE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief 
/// \param CAsyncTcpSocket * socket 
/// \param CAsyncEvent * evt 
/// \return bool
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CAsyncDispatcher::Enqueue(CAsyncTcpSocket* socket, CAsyncEvent* evt)
{
	SafeGuard();

	return PostQueuedCompletionStatus(m_Iocp, 0, reinterpret_cast<ULONG_PTR>(socket), reinterpret_cast<LPOVERLAPPED>(evt->GetTag())) != FALSE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief 
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAsyncDispatcher::Start(unsigned int workerThreadCount)
{
	SafeGuard();

	if (workerThreadCount == 0)
	{
		workerThreadCount = std::thread::hardware_concurrency() * 2;
	}
	
	for (size_t i = 0; i < workerThreadCount; ++i)
	{
		m_WorkerThreadArray.push_back(xnew(CWorkerThread, &m_ShutdownEvent));
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief 
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAsyncDispatcher::Stop()
{
	SafeGuard();

	// 쓰레드 정지
	for (auto& workerThread : m_WorkerThreadArray)
	{
		workerThread->Stop();
	}

	// 실제로 끝날때까지 대기
	for (auto& workerThread : m_WorkerThreadArray)
	{
		workerThread->Join();
	}

	// 메모리 정리
	for (auto& workerThread : m_WorkerThreadArray)
	{
		xdelete(workerThread);
	}
}

CAsyncDispatcher* g_AsyncDispatcher = nullptr;