////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file Memory.cpp
/// \author bjh1371
/// \date 2015/07/02
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Memory.h"

namespace Memory
{
	// 할당 메모리 정보
	struct ALLOC_CONTEXT
	{
		const TCHAR* File;
		const int    Line;

		/// \brief 생성자
		ALLOC_CONTEXT(const TCHAR* f, const int l)
		:
		File(f),
		Line(l)
		{

		}
	};

	// 할당 메모리 정보 맵
	typedef stx::unordered_map<void*, ALLOC_CONTEXT> CAllocMap;
	CAllocMap g_MemoryAllocMap;

	// 메모리 정보 락
	CCriticalSection g_MemoryMapLock;

	/// \brief 메모리 할당 정보 추가
	void AddAllocContext(void* ptr, const TCHAR* file, int line)
	{
#ifdef MemoryCheck
		g_MemoryMapLock.Lock();
		auto itr(g_MemoryAllocMap.find(ptr));
		if (itr == g_MemoryAllocMap.end())
		{
			g_MemoryAllocMap.insert(CAllocMap::value_type(ptr, ALLOC_CONTEXT(file, line)));
		}
		else
		{
			assert(false && "allocate the duplicated memory");
		}
		g_MemoryMapLock.UnLock();
#endif // MemoryCheck
	}

	/// \brief 메모리 할당 정보 삭제
	void RemoveAllocContext(void * ptr)
	{
#ifdef MemoryCheck
		g_MemoryMapLock.Lock();
		auto itr(g_MemoryAllocMap.find(ptr));
		if (itr != g_MemoryAllocMap.end())
		{
			g_MemoryAllocMap.erase(itr);
		}
		else
		{
			assert(false && "delete the pointer does not exist");
		}
		g_MemoryMapLock.UnLock();
#endif // MemoryCheck
	}

	/// \brief 메모리 릭을 리포트한다.
	void ReportMemoryLeak()
	{
		if (!g_MemoryAllocMap.empty())
		{
			CFixedString<64> timeStamp;
			timeStamp.MakeDateTime(_T('-'), _T(' '), _T('-'));
			tofstream writeFile(Generic::FormatSafe(_T("%a Memory Leak Report.txt"), timeStamp.Get()), std::ofstream::out);

			if (writeFile.is_open())
			{
				for (auto& element : g_MemoryAllocMap)
				{
					void* allocPtr = element.first;
					const ALLOC_CONTEXT& allocContext = element.second;
					writeFile << _T("Memory Address : ") << allocPtr << _T("\r\n")
						<<_T("File : ") << allocContext.File << _T("\r\n")
						<<_T("Line : ") << allocContext.Line << _T("\r\n") << _T("\r\n");
				}

				writeFile.close();
			}
		}
	}
}