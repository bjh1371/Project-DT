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
/// \brief IOCP ��Ŀ������
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CAsyncDispatcher::CWorkerThread : public CThread
{
private:
	CAsyncDispatcher::CShutDownEvent* m_Shutdown; ///< �˴ٿ� ��ü


public:
	/// \brief ������
	CWorkerThread(CAsyncDispatcher::CShutDownEvent* shutdown)
	:
	m_Shutdown(shutdown)
	{
		Start();
	}

	virtual ~CWorkerThread() {}


public:
	/// \brief ����
	virtual void Stop()
	{
		SafeGuard();

		// ���� �� ��Ŀ�����尡 ������ ���̶�� ������ ����.
		PostQueuedCompletionStatus(g_AsyncDispatcher->GetIocp(), 0, 0, m_Shutdown);
	}


private:
	/// \brief ���� ������ ����
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

			// �˴ٿ� ��ü�ϰ��
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
/// \brief ������
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
/// \brief �Ҹ���
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
/// \brief IOCP�� �����Ѵ�
/// \param handle �ڵ�
/// \param key Ű
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
/// \brief �̺�Ʈ�� ����ִ´�.
/// \param key Ű
/// \param evt �̺�Ʈ
/// \return bool ��������
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

	// ������ ����
	for (auto& workerThread : m_WorkerThreadArray)
	{
		workerThread->Stop();
	}

	// ������ ���������� ���
	for (auto& workerThread : m_WorkerThreadArray)
	{
		workerThread->Join();
	}

	// �޸� ����
	for (auto& workerThread : m_WorkerThreadArray)
	{
		xdelete(workerThread);
	}
}

CAsyncDispatcher* g_AsyncDispatcher = nullptr;