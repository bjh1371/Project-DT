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
/// \brief WsaBuf를 모아서 보내기위한 SendEvent
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CAsyncTcpSendEvent : public CAsyncEvent
{
private:
	typedef std::array<CAsyncTcpEvent*, 250> CSendEventArray;


	WSABUF*         m_WsaBufArray;  ///< Send할 WsaBufArray
	int             m_CurrentCount; ///< 현재 Pending된 이벤트 숫자
	int             m_MaxCount;     ///< 최대 펜딩 가능 숫자
	int             m_TotalBytes;   ///< 펜딩된 SendEvent 전체 바이트
	CSendEventArray m_SendArray;    ///< 실제 펜딩된 SendEvent 배열


public:
	/// \brief 생성자
	CAsyncTcpSendEvent(int maxCount = 250);
	
	/// \brief 소멸자
	virtual ~CAsyncTcpSendEvent();
	
	
public:
	/// \brief 실행
	virtual void Execute(bool success, DWORD transferred, ULONG_PTR target);


public:
	/// \brief SendEvent를 추가한다.
	bool Add(CAsyncTcpEvent* evt);

	/// \brief 리셋
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