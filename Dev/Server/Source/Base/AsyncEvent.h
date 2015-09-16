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
/// \brief Timer �̺�Ʈ�� Tcp �̺�Ʈ�� �߻�ȭ �ϱ� ���� ���� Ŭ����
///        ���� �Լ��� ���� ���ؼ� OVERLAPPED�� this �����͸� ��� �ִ�.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CAsyncEvent
{
public:
	struct TAG : public OVERLAPPED
	{
		CAsyncEvent* Owner; ///< �ڱ� �ڽ�
	};


private:
	TAG              m_Tag;      ///< IOCP ����� ���� OVERLAPPED ��ü
	std::atomic<int> m_RefCount; ///< ���۷��� ī��Ʈ
	

public:
	/// \brief ������
	CAsyncEvent();
	
	/// \brief �Ҹ���
	virtual ~CAsyncEvent();
	
	
public:
	/// \brief ����
	virtual void Execute(bool success, DWORD transferred, ULONG_PTR target) = 0;


public:
	/// \brief ����ī��Ʈ ����
	int IncreaseRefCount() { return ++m_RefCount; }

	/// \brief ����ī��Ʈ ����
	int DecreaseRefCount() { return --m_RefCount; }


public:
	CAsyncEvent::TAG* GetTag() { return &m_Tag; }
};