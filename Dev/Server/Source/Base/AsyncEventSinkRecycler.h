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
/// \brief 피어 재활용 쓰레드
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CAsyncEventSinkRecycler : public CThread
{
private:
	typedef tbb::concurrent_queue<CAsyncEventSink*> CQueue;

	CQueue        m_Queue;    ///< 리사이클 큐
	volatile bool m_LifeSpan; ///< 수명 주기
	

public:
	/// \brief 생성자
	CAsyncEventSinkRecycler();

	/// \brief 소멸자
	virtual ~CAsyncEventSinkRecycler();


public:
	/// \brief 정지
	virtual void Stop() override;


public:
	/// \brief 추가한다.
	void Add(CAsyncEventSink* sink);


public:
	/// \brief 현재 큐에 있는 가비지 숫자를 반환한다.
	int GetCount();


private:
	/// \brief 실제 작업 함수
	virtual void ThreadMain() override;
};

extern CAsyncEventSinkRecycler* g_AsyncEventSinkRecycler;
