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
		SEND, ///< �������� 
		IDLE, ///< ���
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

		CAsyncTcpEvent* Event;    ///< ���� �̺�Ʈ
		long long       Sequence; ///< Send ������ ���߱� ���� ������
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
	const long long        m_InstanceId;     ///< �ν��Ͻ� ID
	CAsyncTcpSocket*       m_Socket;         ///< ����
	CAsyncTcpEvent*        m_RecvEvent;      ///< Recv �̺�Ʈ
	CAsyncTcpEvent*        m_SendEvent;      ///< 1-Send �̺�Ʈ
	std::atomic<SendState> m_SendState;      ///< Send ����
	CSendEventQueue        m_SendEventQueue; ///< SendEventQueue
	volatile long long     m_SendSequence;   ///< SendSequence
	CAsyncMarShaller*      m_Marshaller;     ///< ��Ŷ ������
	CAsyncPeerListener*    m_Listener;       ///< �� ������ �����ִ� ������


public:
	/// \brief ������
	CAsyncTcpPeer(CAsyncMarShaller* marshaller);

	/// \brief �Ҹ���
	virtual ~CAsyncTcpPeer();


public:
	/// \brief Send �̺�Ʈ ���
	void PostSend(CAsyncTcpEvent* evt);

	/// \brief Recv �̺�Ʈ ���
	void PostRecv(int offset = 0);
	
	/// \brief Connect �̺�Ʈ ���
	void PostConnect(const TCHAR* address, short port);

	/// \brief Disconnect �̺�Ʈ ���
	void PostDisconnect(CAsyncTcpEvent::DisconnectReason dr);


public:
	/// \brief ���ӵǾ��ִ°�?
	bool IsConnected() { return m_Socket ? m_Socket->IsConnected() : false; }

	/// \brief ������ �����Ѵ�.
	void ResetConnectEx();


public:
	long long GetInstanceId() const { return m_InstanceId; }
	
	CAsyncPeerListener* GetListener() const { return m_Listener; }
	void SetListener(CAsyncPeerListener* value) { m_Listener = value; }

	CAsyncTcpSocket* GetSocket() const { return m_Socket; }
	void SetSocket(CAsyncTcpSocket* val) { m_Socket = val; }


protected:
	/// \brief Connect�� ȣ��Ǵ� �Լ�
	virtual void OnConnected();

	/// \brief Disconnecct�� ȣ��Ǵ� �Լ�
	virtual void OnDisconnected();


private:
		/// \brief SendQueue�� �ִ� �̺�Ʈ���� ������ ����
	int FlushSendQueue(CWrapperEvent* evt = nullptr);


private:
	/// \brief Receive �̺�Ʈ ������ ȣ��
	void OnRecvEvent(bool success, DWORD transferred, CAsyncTcpEvent* evt);

	/// \brief Send �̺�Ʈ ����� ȣ��
	void OnSendEvent(bool success, DWORD transferred, CAsyncTcpEvent* evt);


private:
	/// \brief ��ü �ı�
	virtual void DestroyInternal() override;

	/// \brief ��ü ����
	void Reset();
};
