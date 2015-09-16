////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file AsyncTimerEvent.h
/// \author bjh1371
/// \date 2015/07/02
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "AsyncEvent.h"

class CAsyncEventSink;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \class CAsyncTimerEvent
/// \brief 
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CAsyncTimerEvent : public CAsyncEvent
{
private:
	CAsyncEventSink* m_Sink;       ///< 이벤트를 던진 이벤트 싱크
	Milli_t          m_Activation; ///< 실행시간
	Milli_t          m_Expiration; ///< 만료시간
	Milli_t          m_Interval;   ///< 간격
	

public:
	/// \brief 생성자
	CAsyncTimerEvent();
	
	/// \brief 소멸자
	virtual ~CAsyncTimerEvent();
	
	
public:
	/// \brief 실행
	virtual void Execute(bool success, DWORD transferred, ULONG_PTR target);

	/// \brief 정지 다음 틱이 되면 정지된다.
	void Stop();


public:
	CAsyncEventSink* GetSink() const { return m_Sink; }
	void SetSink(CAsyncEventSink* val) { m_Sink = val; }

	Milli_t GetActivation() const { return m_Activation; }
	void SetActivation(Milli_t val) { m_Activation = val; }
	
	Milli_t GetExpiration() const { return m_Expiration; }
	void SetExpiration(Milli_t val) { m_Expiration = val; }
	
	Milli_t GetInterval() const { return m_Interval; }
	void SetInterval(Milli_t val) { m_Interval = val; }
};