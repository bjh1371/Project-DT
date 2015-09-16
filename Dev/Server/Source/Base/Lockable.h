////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file Lockable.h
/// \author bjh1371
/// \date 2014.11.7
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "NoneCopyable.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \class CLockable
/// \brief 
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CLockable : private CNoneCopyable
{
private:
	DWORD m_OwingThreadIndex; ///< ���� �� ������ ������ ID


public:
	/// \brief ������
	CLockable(): m_OwingThreadIndex(0) { }

	/// \brief �Ҹ���
	virtual ~CLockable() { }


public:
	/// \brief ��
	virtual void Lock() = 0;

	/// \brief ���
	virtual void UnLock() = 0;


protected:
	/// \brief �� �ɱ��� 
	void BeforeLock();

	/// \brief �� �� ��
	void AfterLock();

	///< �� Ǭ ��
	void AfterUnLock();

	///< �� Ǯ�� ��
	void BeforeUnLock();


public:
	template <typename Predicate>
	void Synchronize(Predicate pred)
	{
		Lock();
		pred();
		UnLock();
	}
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \class CBasicCriticalSection
/// \brief ũ��Ƽ�� ���� ĸ��ȭ
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CBasicCriticalSection
{
private:
	CRITICAL_SECTION m_Lock; ///< ũ��Ƽ�ü��� ��ü


public:
	/// \brief ������
	CBasicCriticalSection()
	{
		InitializeCriticalSection(&m_Lock);
	}

	/// \brief �Ҹ���
	~CBasicCriticalSection()
	{
		DeleteCriticalSection(&m_Lock);
	}


public:
	/// \brief ��
	void Lock()
	{
		EnterCriticalSection(&m_Lock);
	}

	/// \brief ���
	void UnLock()
	{
		LeaveCriticalSection(&m_Lock);
	}
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \class CReenterCriticalSection
/// \brief ������ ������ ũ��Ƽ�� ����
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CReenterCriticalSection : public CLockable
{
private:
	CBasicCriticalSection m_Lock; ///< ��


public:
	/// \brief ������
	CReenterCriticalSection()
	{
		
	}

	/// \brief �Ҹ���
	virtual ~CReenterCriticalSection()
	{
		
	}


public:
	/// \brief ��
	virtual void Lock()
	{
		m_Lock.Lock();
	}

	/// \brief ���
	virtual void UnLock()
	{
		m_Lock.UnLock();
	}
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \class CCriticalSection
/// \brief 
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CCriticalSection : public CLockable
{
private:
	CBasicCriticalSection m_Lock; ///< ��


public:
	/// \brief ������
	CCriticalSection() {}

	/// \brief �Ҹ���
	virtual ~CCriticalSection() {}


public:
	/// \brief ��
	virtual void Lock()
	{
		BeforeLock();
		m_Lock.Lock();
		AfterLock();
	}

	/// \brief ���
	virtual void UnLock()
	{
		BeforeUnLock();
		m_Lock.UnLock();
		AfterUnLock();
	}
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \class CReaderWriterLock
/// \brief ���� �б� ��
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CReaderWriterLock : private CNoneCopyable
{
private:
	SRWLOCK m_Lock; ///< �б� ���� ��


public:
	/// \brief ������
	CReaderWriterLock() { InitializeSRWLock(&m_Lock); }

	/// \brief �Ҹ���
	virtual ~CReaderWriterLock() {}
	

public:
	/// \brief Read ��
	void LockForRead() { AcquireSRWLockShared(&m_Lock); }

	/// \brief Read ���
	void ReleaseForRead() { ReleaseSRWLockShared(&m_Lock); }


public:
	/// \brief Write ��
	void LockForWrite() { AcquireSRWLockExclusive(&m_Lock); }

	/// \brief Write ���
	void ReleaseForWrite() { ReleaseSRWLockExclusive(&m_Lock); }


public:
	/// \brief �б� �� 
	template <typename Pred>
	void SynchronizeForRead(Pred pred)
	{
		LockForRead();
		pred();
		ReleaseForRead();
	}

	/// \brief ���� ��
	template <typename Pred>
	void SynchronizeForWrite(Pred pred)
	{
		LockForWrite();
		pred();
		ReleaseForWrite();
	}
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \class CScopeLock
/// \brief 
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CScopeLock
{
private:
	CLockable& m_Lock;


public:
	/// \brief ������
	CScopeLock(CLockable& lock)
	:
	m_Lock(lock)
	{
		m_Lock.Lock();
	}

	/// \brief �Ҹ���
	~CScopeLock()
	{
		m_Lock.UnLock();
	}
};