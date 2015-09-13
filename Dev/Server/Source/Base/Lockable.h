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
	DWORD m_OwingThreadIndex; ///< 현재 락 점유한 쓰레드 ID


public:
	/// \brief 생성자
	CLockable(): m_OwingThreadIndex(0) { }

	/// \brief 소멸자
	virtual ~CLockable() { }


public:
	/// \brief 락
	virtual void Lock() = 0;

	/// \brief 언락
	virtual void UnLock() = 0;


protected:
	/// \brief 락 걸기전 
	void BeforeLock();

	/// \brief 락 건 후
	void AfterLock();

	///< 락 푼 후
	void AfterUnLock();

	///< 락 풀고 전
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
/// \brief 크리티컬 섹션 캡슐화
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CBasicCriticalSection
{
private:
	CRITICAL_SECTION m_Lock; ///< 크리티컬섹션 객체


public:
	/// \brief 생성자
	CBasicCriticalSection()
	{
		InitializeCriticalSection(&m_Lock);
	}

	/// \brief 소멸자
	~CBasicCriticalSection()
	{
		DeleteCriticalSection(&m_Lock);
	}


public:
	/// \brief 락
	void Lock()
	{
		EnterCriticalSection(&m_Lock);
	}

	/// \brief 언락
	void UnLock()
	{
		LeaveCriticalSection(&m_Lock);
	}
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \class CReenterCriticalSection
/// \brief 리엔터 가능한 크리티컬 섹션
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CReenterCriticalSection : public CLockable
{
private:
	CBasicCriticalSection m_Lock; ///< 락


public:
	/// \brief 생성자
	CReenterCriticalSection()
	{
		
	}

	/// \brief 소멸자
	virtual ~CReenterCriticalSection()
	{
		
	}


public:
	/// \brief 락
	virtual void Lock()
	{
		m_Lock.Lock();
	}

	/// \brief 언락
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
	CBasicCriticalSection m_Lock; ///< 락


public:
	/// \brief 생성자
	CCriticalSection() {}

	/// \brief 소멸자
	virtual ~CCriticalSection() {}


public:
	/// \brief 락
	virtual void Lock()
	{
		BeforeLock();
		m_Lock.Lock();
		AfterLock();
	}

	/// \brief 언락
	virtual void UnLock()
	{
		BeforeUnLock();
		m_Lock.UnLock();
		AfterUnLock();
	}
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \class CReaderWriterLock
/// \brief 쓰기 읽기 락
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CReaderWriterLock : private CNoneCopyable
{
private:
	SRWLOCK m_Lock; ///< 읽기 쓰기 락


public:
	/// \brief 생성자
	CReaderWriterLock() { InitializeSRWLock(&m_Lock); }

	/// \brief 소멸자
	virtual ~CReaderWriterLock() {}
	

public:
	/// \brief Read 락
	void LockForRead() { AcquireSRWLockShared(&m_Lock); }

	/// \brief Read 언락
	void ReleaseForRead() { ReleaseSRWLockShared(&m_Lock); }


public:
	/// \brief Write 락
	void LockForWrite() { AcquireSRWLockExclusive(&m_Lock); }

	/// \brief Write 언락
	void ReleaseForWrite() { ReleaseSRWLockExclusive(&m_Lock); }


public:
	/// \brief 읽기 락 
	template <typename Pred>
	void SynchronizeForRead(Pred pred)
	{
		LockForRead();
		pred();
		ReleaseForRead();
	}

	/// \brief 쓰기 락
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
	/// \brief 생성자
	CScopeLock(CLockable& lock)
	:
	m_Lock(lock)
	{
		m_Lock.Lock();
	}

	/// \brief 소멸자
	~CScopeLock()
	{
		m_Lock.UnLock();
	}
};