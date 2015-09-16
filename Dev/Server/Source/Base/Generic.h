////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file Generic.h
/// \author bjh1371
/// \date 2015/07/03
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

namespace Generic
{
	/// \brief 포인터 삭제
	template <typename T>
	void SafeDelete(T*& ptr)
	{
		if (ptr)
		{
			xdelete(ptr);
			ptr = nullptr;
		}
	}

	/// \brief 배열 삭제
	template <typename T>
	void SafeDeleteArray(T*& ptr)
	{
		if (ptr)
		{
			xdelete_arry(ptr);
			ptr = nullptr;
		}
	}

	/// \brief 포맷 형식의 로그를 작성한다.
	template <typename... Arguments>
	LPCTSTR FormatSafe(LPCTSTR fmt, const Arguments&... arg)
	{
		CFixedString<2048> fixedString;
		return fixedString.FormatSafe(fmt, arg...);
	}
}