////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file CAsyncTcpPeer.h
/// \author 
/// \date 2014.10.24
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "AsyncTcpEvent.h"
#include "AsyncEventSink.h"
#include "AsyncTcpSocket.h"

class CAsyncPeerListener;
class CAsyncMarShaller;
class CAsyncDispatcher;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \class CSession
/// \brief 
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CAsyncTcpPeer : public CAsyncEventSink
{
	friend class CAsyncTcpSocket;


private:
	enum SendState
	{
		SEND, ///< 보내는중 
		IDLE, ///< 대기
	};

	struct CWrapperEvent
	{
		CWrapperEvent(CAsyncTcpEvent* evt, long long s)
		:
		Event(evt),
		Sequence(s)
		{
			Event->IncreaseRefCount();
		}

		~CWrapperEvent()
		{
			if (Event->DecreaseRefCount() == 0)
			{
				CAsyncTcpEvent::Dealloc(Event);
			}
		}

		CAsyncTcpEvent* Event;    ///< 실제 이벤트
		long long       Sequence; ///< Send 순서를 맞추기 위한 시퀀스
	};

	struct COMPARE
	{
		bool operator() (const CWrapperEvent* lhs, const CWrapperEvent* rhs)
		{
			return lhs->Sequence > rhs->Sequence;
		}
	};

	typedef tbb::concurrent_priority_queue<CWrapperEvent*, COMPARE> CSendEventQueue;


private:
	const long long        m_InstanceId;     ///< 인스턴스 ID
	CAsyncTcpSocket*       m_Socket;         ///< 소켓
	CAsyncTcpEvent*        m_RecvEvent;      ///< Recv 이벤트
	CAsyncTcpEvent*        m_SendEvent;      ///< 1-Send 이벤트
	std::atomic<SendState> m_SendState;      ///< Send 상태
	CSendEventQueue        m_SendEventQueue; ///< SendEventQueue
	volatile long long     m_SendSequence;   ///< SendSequence
	CAsyncMarShaller*      m_Marshaller;     ///< 패킷 마샬러
	CAsyncPeerListener*    m_Listener;       ///< 이 세션이 속해있는 리스너


public:
	/// \brief 생성자
	CAsyncTcpPeer(CAsyncMarShaller* marshaller);

	/// \brief 소멸자
	virtual ~CAsyncTcpPeer();


public:
	/// \brief Send 이벤트 등록
	void PostSend(CAsyncTcpEvent* evt);

	/// \brief Recv 이벤트 등록
	void PostRecv(int offset = 0);
	
	/// \brief Connect 이벤트 등록
	void PostConnect(const TCHAR* address, short port);

	/// \brief Disconnect 이벤트 등록
	void PostDisconnect(CAsyncTcpEvent::DisconnectReason dr);


public:
	/// \brief 접속되어있는가?
	bool IsConnected() { return m_Socket ? m_Socket->IsConnected() : false; }

	/// \brief 소켓을 리셋한다.
	void ResetConnectEx();


public:
	long long GetInstanceId() const { return m_InstanceId; }
	
	CAsyncPeerListener* GetListener() const { return m_Listener; }
	void SetListener(CAsyncPeerListener* value) { m_Listener = value; }

	CAsyncTcpSocket* GetSocket() const { return m_Socket; }
	void SetSocket(CAsyncTcpSocket* val) { m_Socket = val; }


protected:
	/// \brief Connect시 호출되는 함수
	virtual void OnConnected();

	/// \brief Disconnecct시 호출되는 함수
	virtual void OnDisconnected();


private:
		/// \brief SendQueue에 있는 이벤트들을 실제로 전송
	int FlushSendQueue(CWrapperEvent* evt = nullptr);


private:
	/// \brief Receive 이벤트 종류시 호출
	void OnRecvEvent(bool success, DWORD transferred, CAsyncTcpEvent* evt);

	/// \brief Send 이벤트 종료시 호출
	void OnSendEvent(bool success, DWORD transferred, CAsyncTcpEvent* evt);


private:
	/// \brief 객체 파괴
	virtual void DestroyInternal() override;

	/// \brief 객체 리셋
	void Reset();
};
