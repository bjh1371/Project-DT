////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file CAsyncRecycler.h
/// \author 
/// \date 2014.11.3
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Thread.h"

#include "AsyncEventSink.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \class CAsyncRecycler
/// \brief �Ǿ� ��Ȱ�� ������
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CAsyncEventSinkRecycler : public CThread
{
private:
	typedef tbb::concurrent_queue<CAsyncEventSink*> CQueue;

	CQueue        m_Queue;    ///< ������Ŭ ť
	volatile bool m_LifeSpan; ///< ���� �ֱ�
	

public:
	/// \brief ������
	CAsyncEventSinkRecycler();

	/// \brief �Ҹ���
	virtual ~CAsyncEventSinkRecycler();


public:
	/// \brief ����
	virtual void Stop() override;


public:
	/// \brief �߰��Ѵ�.
	void Add(CAsyncEventSink* sink);


public:
	/// \brief ���� ť�� �ִ� ������ ���ڸ� ��ȯ�Ѵ�.
	int GetCount();


private:
	/// \brief ���� �۾� �Լ�
	virtual void ThreadMain() override;
};

extern CAsyncEventSinkRecycler* g_AsyncEventSinkRecycler;
