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
	CAsyncEventSink* m_Sink;       ///< �̺�Ʈ�� ���� �̺�Ʈ ��ũ
	Milli_t          m_Activation; ///< ����ð�
	Milli_t          m_Expiration; ///< ����ð�
	Milli_t          m_Interval;   ///< ����
	

public:
	/// \brief ������
	CAsyncTimerEvent();
	
	/// \brief �Ҹ���
	virtual ~CAsyncTimerEvent();
	
	
public:
	/// \brief ����
	virtual void Execute(bool success, DWORD transferred, ULONG_PTR target);

	/// \brief ���� ���� ƽ�� �Ǹ� �����ȴ�.
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