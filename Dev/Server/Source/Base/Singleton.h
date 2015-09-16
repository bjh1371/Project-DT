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
	static CCriticalSection m_Lock;     ///< ��
	static std::atomic<T*>  m_Instance; ///< �ν��Ͻ� ������
	

protected:
	/// \brief ������
	CSingleton()
	{
		BootStrap::AddExitFunction(Clear);
	}
	
	/// \brief �Ҹ���
	virtual ~CSingleton() { };

	
public:
	/// \brief �ν��Ͻ��� ��ȯ�Ѵ�.
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

	/// \brief ��ü �ʱ�ȭ
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
    