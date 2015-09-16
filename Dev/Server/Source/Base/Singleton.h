////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file Singleton.h
/// \author bjh1371
/// \date 2015/02/15
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Lockable.h"
#include "BootStrap.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \class CSingleton
/// \brief 
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template <typename T>
class CSingleton
{
private:
	static CCriticalSection m_Lock;     ///< 락
	static std::atomic<T*>  m_Instance; ///< 인스턴스 포인터
	

protected:
	/// \brief 생성자
	CSingleton()
	{
		BootStrap::AddExitFunction(Clear);
	}
	
	/// \brief 소멸자
	virtual ~CSingleton() { };

	
public:
	/// \brief 인스턴스를 반환한다.
	static T* GetInstance()
	{
		T* instance = m_Instance.load();
		if (instance == nullptr)
		{
			CScopeLock lock(m_Lock);
			instance = m_Instance.load();
			if (instance == nullptr)
			{
				instance = new T();
				m_Instance.store(instance);
			}
		}
		return instance;
	}

	/// \brief 객체 초기화
	static void Clear()
	{
		T* instance = m_Instance.load();
		if (instance != nullptr)
		{
			CScopeLock lock(m_Lock);
			instance = m_Instance.load();
			if (instance != nullptr)
			{
				delete instance;
			}
		}
	}
	
	
private:
	CSingleton(CSingleton& rhs);
	CSingleton& operator=(CSingleton& rhs);
};

template <typename T> std::atomic<T*> CSingleton<T>::m_Instance = nullptr;
template <typename T> CCriticalSection CSingleton<T>::m_Lock;

#define DECLARE_SINGLETON_INTERFACE(CLASS) \
    friend class CSingleton<CLASS>; \
	private : \
	CLASS(); \
	virtual ~CLASS();
    