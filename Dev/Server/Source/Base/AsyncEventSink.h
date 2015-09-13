////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file CAsyncEventSink.h
/// \author 
/// \date 2014.11.5
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "RefCountable.h"

class CAsyncTimerEvent;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \class CAsyncEventSink
/// \brief Ÿ�̸� �̺�Ʈ�� �������ִ� Base��ü
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CAsyncEventSink : public CRefCountable
{
private:
	std::atomic<bool> m_PendingDeletion; ///< ���� ������ΰ�?


public:
	/// \brief ������
	CAsyncEventSink();

	/// \brief �Ҹ���
	virtual ~CAsyncEventSink();


public:
	/// \brief ���� ����
	bool ExChangePendingDeletion(bool value) { return m_PendingDeletion.exchange(value); }

	/// \brief ���� ������ΰ�?
	bool IsPendingDeletion() { return m_PendingDeletion; }

	/// \brief ��ü ����
	void Destroy();


public:
	/// \brief Timer�� ȣ��Ǵ� �Լ�
	virtual void OnTimer(CAsyncTimerEvent* evt);


private:
	/// \brief ���� ��ü ����
	virtual void DestroyInternal();
};
