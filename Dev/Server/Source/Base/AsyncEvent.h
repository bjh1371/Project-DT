////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file AsyncEvent.h
/// \author bjh1371
/// \date 2015/07/02
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "RefCountable.h"

class CAsyncEventSink;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \class CAsyncEvent
/// \brief Timer 이벤트와 Tcp 이벤트를 추상화 하기 위해 만든 클래스
///        가상 함수를 쓰기 위해서 OVERLAPPED가 this 포인터를 들고 있다.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CAsyncEvent
{
public:
	struct TAG : public OVERLAPPED
	{
		CAsyncEvent* Owner; ///< 자기 자신
	};


private:
	TAG              m_Tag;      ///< IOCP 사용을 위한 OVERLAPPED 객체
	std::atomic<int> m_RefCount; ///< 레퍼런스 카운트
	

public:
	/// \brief 생성자
	CAsyncEvent();
	
	/// \brief 소멸자
	virtual ~CAsyncEvent();
	
	
public:
	/// \brief 실행
	virtual void Execute(bool success, DWORD transferred, ULONG_PTR target) = 0;


public:
	/// \brief 레퍼카운트 증가
	int IncreaseRefCount() { return ++m_RefCount; }

	/// \brief 레퍼카운트 감소
	int DecreaseRefCount() { return --m_RefCount; }


public:
	CAsyncEvent::TAG* GetTag() { return &m_Tag; }
};