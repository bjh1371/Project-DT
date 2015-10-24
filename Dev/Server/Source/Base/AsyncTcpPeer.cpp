////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file CAsyncTcpPeer.cpp
/// \author 
/// \date 2014.10.24
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "AsyncTcpPeer.h"

#include "AsyncDispatcher.h"
#include "AsyncEventSinkRecycler.h"
#include "AsyncMarshaller.h"
#include "AsyncPeerListener.h"
#include "AsyncTcpSocketPool.h"

namespace
{
	volatile long long g_SessionRegistry = 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief 생성자
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CAsyncTcpPeer::CAsyncTcpPeer(CAsyncMarShaller* marshaller)
:
m_InstanceId(_InterlockedIncrement64(&g_SessionRegistry)),
m_Socket(nullptr),
m_RecvEvent(nullptr),
m_SendEvent(nullptr),
m_SendState(IDLE),
m_SendEventQueue(),
m_SendSequence(0),
m_Marshaller(marshaller),
m_Listener(nullptr)
{
	SafeGuard();

	m_RecvEvent = xnew(CAsyncTcpEvent, CAsyncTcpEvent::MAX_CAPACITY);
	m_SendEvent = xnew(CAsyncTcpEvent, CAsyncTcpEvent::MAX_CAPACITY);

	// 최초 레퍼카운트 증가
	m_RecvEvent->IncreaseRefCount();
	m_SendEvent->IncreaseRefCount();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief 소멸자
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CAsyncTcpPeer::~CAsyncTcpPeer()
{
	SafeGuard();

	Generic::SafeDelete(m_Marshaller);

	if (m_RecvEvent)
	{
		if (m_RecvEvent->DecreaseRefCount() == 0)
		{
			Generic::SafeDelete(m_RecvEvent);
		}
		else
		{
			assert(false && "can not delete recv event");
		}
	}

	if (m_SendEvent)
	{
		if (m_SendEvent->DecreaseRefCount() == 0)
		{
			Generic::SafeDelete(m_SendEvent);
		}
		else
		{
			assert(false && "can not delete send event");
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Send이벤트를 포스트한다.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAsyncTcpPeer::PostSend(CAsyncTcpEvent* evt)
{
	SafeGuard();

	if (IsConnected())
	{
		if (m_Marshaller)
		{
			m_Marshaller->Marshall(evt->m_Buf, evt->m_TotalBytes);
		}
		
		if (CWrapperEvent* wrapper = xnew(CWrapperEvent, evt, InterlockedIncrement64(&m_SendSequence)))
		{
			SendState state = IDLE;
			if (m_SendState.compare_exchange_weak(state, SEND))
			{
				int written = FlushSendQueue(wrapper);

				// 접속 중이라면 최소 방금 패킷은 보내야된다.
				if (IsConnected())
				{
					if (written == 0)
					{
						assert(false && "flush error");
					}
				}
			}
			else if (state == SEND)
			{
				m_SendEventQueue.push(wrapper);
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Recv이벤트를 포스트한다.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAsyncTcpPeer::PostRecv(int offset /*= 0*/)
{
	SafeGuard();

	if (IsConnected())
	{
		IncreaseRefCount(CRefCountable::REFCOUNT_RECV);
		m_Socket->PostRecv(m_RecvEvent, offset);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Connect 이벤트 등록
/// \param address 접속할 주소
/// \param port 포트
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAsyncTcpPeer::PostConnect(const TCHAR* address, short port)
{
	SafeGuard();

	IncreaseRefCount(CRefCountable::REFCOUNT_CONNECT);

	assert(m_Socket == nullptr && "socket must null");

	// 소켓하나를 가져오고
	m_Socket = g_AsyncTcpSocketPool->Pop();
	m_Socket->PostConnect(address, port);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief 접속을 종료시킨다.
/// \param dr 접속종료 이유
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAsyncTcpPeer::PostDisconnect(CAsyncTcpEvent::DisconnectReason dr)
{
	SafeGuard();

	IncreaseRefCount(CRefCountable::REFCOUNT_DISCONNECT);

	if (m_Socket && m_Socket->IsConnected())
	{
		m_Socket->PostDisconnect(dr);
	}

	DecreaseRefCount(CRefCountable::REFCOUNT_DISCONNECT);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief 접속후 호출
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAsyncTcpPeer::OnConnected()
{
	SafeGuard();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief 접속 종료 후 호출
/// \param dr 접속종료 이유
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAsyncTcpPeer::OnDisconnected()
{
	SafeGuard();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief 
/// \param CWrapperEvent * evt 
/// \return int
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CAsyncTcpPeer::FlushSendQueue(CWrapperEvent* evt /*= nullptr*/)
{
	SafeGuard();

	IncreaseRefCount(CRefCountable::REFCOUNT_FLUSHQUEUE);
	
	int written = 0;
	if (IsConnected())
	{
		if (evt != nullptr)
		{
			m_SendEventQueue.push(evt);
		}

		int remain = m_SendEvent->m_Capacity - m_SendEvent->m_TotalBytes;
		while (remain > 0)
		{
			CWrapperEvent* wrapper = nullptr;
			if (m_SendEventQueue.try_pop(wrapper))
			{
				CAsyncTcpEvent* evt = wrapper->Event;
				if (remain < evt->m_TotalBytes)
				{
					// 처리할수 있는 크기보다 더 큰 이벤트는 다시 나중에 처리
					// 나중에 순서를 맞추기 위해 시퀀스가 필요
					m_SendEventQueue.push(wrapper);
					break;
				}
				else
				{
					remain -= evt->m_TotalBytes;
					written += evt->m_TotalBytes;
					memcpy(m_SendEvent->m_Buf + m_SendEvent->m_TotalBytes, evt->m_Buf, evt->m_TotalBytes);
					m_SendEvent->m_TotalBytes += evt->m_TotalBytes;

					xdelete(wrapper);
				}
			}
			else
				break;
		}

		if (written > 0)
		{
			IncreaseRefCount(CRefCountable::REFCOUNT_SEND);
			m_Socket->PostSend(m_SendEvent, m_SendEvent->m_TotalBytes);
		}
	}

	DecreaseRefCount(CRefCountable::REFCOUNT_FLUSHQUEUE);
	
	return written;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Recv 이벤트 종료시 호출
/// \param success 성공여부
/// \param transferred 받은 양
/// \param evt 이벤트
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAsyncTcpPeer::OnRecvEvent(bool success, DWORD transferred, CAsyncTcpEvent* evt)
{
	SafeGuard();

	assert(evt == m_RecvEvent && "recv event is invalid");

	if (success && transferred > 0)
	{
		assert(evt->m_TotalBytes + transferred <= CAsyncTcpEvent::MAX_CAPACITY && "buffer overflow");

		int total = evt->m_TotalBytes + transferred;
		int handled = 0;
		while (handled < total)
		{
			// 들어온 내용 처리
			if (m_Marshaller)
			{
				int result = m_Marshaller->UnMarshall(this, m_RecvEvent->m_Buf + handled, total - handled);
				if (result == 0)
					break;

				else
					handled += result;
			}
			// 들어온 내용 무시
			else
				handled = total;
		}

		// 찌꺼기가 조금 남은경우
		if (handled > 0 && handled < total)
		{
			memmove(m_RecvEvent->m_Buf, m_RecvEvent->m_Buf + handled, total - handled);
		}

		// 다시 Recv 포스트
		PostRecv(total - handled);
	}
	else
	{
		PostDisconnect(CAsyncTcpEvent::DR_RECV_FAIL);
	}

	// Post한 레퍼카운트 감소시키고
	DecreaseRefCount(CAsyncEventSink::REFCOUNT_RECV);
	evt->DecreaseRefCount();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Send 이벤트 종료시 호출
/// \param transferred Send 한 데이터
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAsyncTcpPeer::OnSendEvent(bool success, DWORD transferred, CAsyncTcpEvent* evt)
{
	SafeGuard();

	assert(evt == m_SendEvent && "sendevent is invalid");
	if (success && transferred > 0)
	{
		m_SendEvent->m_TotalBytes -= transferred;
		int written = FlushSendQueue();

		if (written == 0)
		{
			SendState state = SEND;
			if (!m_SendState.compare_exchange_weak(state, IDLE))
			{
				assert(false && "send state is invalid");
			}
		}
	}
	
	DecreaseRefCount(CRefCountable::REFCOUNT_SEND);
	evt->DecreaseRefCount();

	if (!success)
	{
		PostDisconnect(CAsyncTcpEvent::DR_SEND_FAIL);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief 객체 파괴
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAsyncTcpPeer::DestroyInternal()
{
	SafeGuard();

	// 객체 리셋 시키고
	Reset();

	// 리스너 큐에 다시 넣는다.
	if (m_Listener)
		m_Listener->PushPeer(this);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief 
/// \return void
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAsyncTcpPeer::Reset()
{
	SafeGuard();

	// Event들 초기화
	m_RecvEvent->Reset();
	m_SendEvent->Reset();

	m_SendState = IDLE;
	m_SendSequence = 0;

	m_Socket = nullptr;
	
	// 대기중이던 이벤트 삭제
	CWrapperEvent* evt = nullptr;
	while (m_SendEventQueue.try_pop(evt))
	{
		Generic::SafeDelete(evt);
	}
}
