////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file AsyncTcpSendEvent.h
/// \author bjh1371
/// \date 2015/09/13
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "AsyncEvent.h"
#include <array>

class CAsyncTcpEvent;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \class CAsyncTcpSendEvent
/// \brief WsaBuf�� ��Ƽ� ���������� SendEvent
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CAsyncTcpSendEvent : public CAsyncEvent
{
private:
	typedef std::array<CAsyncTcpEvent*, 250> CSendEventArray;


	WSABUF*         m_WsaBufArray;  ///< Send�� WsaBufArray
	int             m_CurrentCount; ///< ���� Pending�� �̺�Ʈ ����
	int             m_MaxCount;     ///< �ִ� ��� ���� ����
	int             m_TotalBytes;   ///< ����� SendEvent ��ü ����Ʈ
	CSendEventArray m_SendArray;    ///< ���� ����� SendEvent �迭


public:
	/// \brief ������
	CAsyncTcpSendEvent(int maxCount = 250);
	
	/// \brief �Ҹ���
	virtual ~CAsyncTcpSendEvent();
	
	
public:
	/// \brief ����
	virtual void Execute(bool success, DWORD transferred, ULONG_PTR target);


public:
	/// \brief SendEvent�� �߰��Ѵ�.
	bool Add(CAsyncTcpEvent* evt);

	/// \brief ����
	void Clear();


public:
	WSABUF* GetWsaBufArray() const { return m_WsaBufArray; }
	void SetWsaBufArray(WSABUF* val) { m_WsaBufArray = val; }

	int GetCurrentCount() const { return m_CurrentCount; }
	void SetCurrentCount(int val) { m_CurrentCount = val; }

	int GetMaxCount() const { return m_MaxCount; }
	void SetMaxCount(int val) { m_MaxCount = val; }

	int GetTotalBytes() const { return m_TotalBytes; }
	void SetTotalBytes(int val) { m_TotalBytes = val; }

	CAsyncTcpSendEvent::CSendEventArray GetSendArray() const { return m_SendArray; }
	void SetSendArray(CAsyncTcpSendEvent::CSendEventArray val) { m_SendArray = val; }
};