////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file Memory.h
/// \author bjh1371
/// \date 2015/07/02
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Lockable.h"

// �޸� �Ҵ� ���� üũ
#define MemoryCheck

namespace Memory
{
	/// \brief �޸� �Ҵ� ������ �����ϴ� new
	template <typename T, typename ...Arguments>
	T* New(const TCHAR* file, int line, Arguments&&...args)
	{
		return new(file, line)T(std::forward<Arguments>(args)...);
	}

	/// \brief �޸� �Ҵ� ������ �����ϴ� new �迭
	template <typename T>
	T* NewArray(const TCHAR* file, int line, size_t count)
	{
		return new(file, line)T[count];
	}

	template <typename T>
	void Destroy(const TCHAR* file, int line, T* ptr)
	{
		if (ptr)
		{
			RemoveAllocContext(ptr);

			ptr->~T();
			scalable_free(ptr);
		}
	}

	template <typename T>
	void DestoryArray(T*& ptr)
	{
		if (ptr)
		{
			delete[] ptr;
		}
	}

	/// \brief �޸��Ҵ� ������ �߰��Ѵ�.
	void AddAllocContext(void* ptr, const TCHAR* file, int line);

	/// \brief �޸� �Ҵ� ������ �蠫����.
	void RemoveAllocContext(void * ptr);

	/// \brief �޸� ���� �α��Ѵ�.
	void ReportMemoryLeak();
}

using namespace Memory;

#define xnew(TYPE, ...)  Memory::New<TYPE>(_T(__FILE__), __LINE__,  __VA_ARGS__)
#define xnew_array(TYPE, COUNT) Memory::NewArray<TYPE>(_T(__FILE__), __LINE__, COUNT)

#define xdelete(ptr) Memory::Destroy(_T(__FILE__), __LINE__, ptr)
#define xdelete_arry(ptr) Memory::DestoryArray(ptr)


inline void* operator new(size_t size, const TCHAR* file, int line)
{
	void* memory = scalable_malloc(size);

	AddAllocContext(memory, file, line);

	return memory;
}

inline void operator delete(void* p, const TCHAR* file, int line)
{
	RemoveAllocContext(p);

	scalable_free(p);
}

inline void*  operator new[](size_t size)
{
	void* memory = scalable_malloc(size);
	AddAllocContext(memory, _T("none user allocated"), 0);

	return memory;
}

inline void operator delete[](void* p)
{
	RemoveAllocContext(p);

	scalable_free(p);
}