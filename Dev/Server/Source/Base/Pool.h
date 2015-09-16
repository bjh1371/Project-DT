////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file Pool.cpp
/// \author bjh1371
/// \date 2015/09/01
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \class CPool
/// \brief 
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template <typename T>
class CPool
{
private:
	typedef tbb::concurrent_queue<T*> CQueue;

	CQueue m_Queue; ///< 큐


public:
	/// \brief 생성자
	CPool();

	/// \brief 소멸자
	virtual ~CPool();


public:
	/// \brief 메모리 할당
	T* Alloc()
	{
		T* object = nullptr;
		if (!m_Queue.try_pop(object))
		{
			T* object = xnew(T);
		}

		return object;
	}

	/// \brief 자원 회수
	void Dealloc(T* object)
	{

	}
};