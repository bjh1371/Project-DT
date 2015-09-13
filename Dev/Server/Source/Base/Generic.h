////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file Generic.h
/// \author bjh1371
/// \date 2015/07/03
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

namespace Generic
{
	/// \brief ������ ����
	template <typename T>
	void SafeDelete(T*& ptr)
	{
		if (ptr)
		{
			xdelete(ptr);
			ptr = nullptr;
		}
	}

	/// \brief �迭 ����
	template <typename T>
	void SafeDeleteArray(T*& ptr)
	{
		if (ptr)
		{
			xdelete_arry(ptr);
			ptr = nullptr;
		}
	}

	/// \brief ���� ������ �α׸� �ۼ��Ѵ�.
	template <typename... Arguments>
	LPCTSTR FormatSafe(LPCTSTR fmt, const Arguments&... arg)
	{
		CFixedString<2048> fixedString;
		return fixedString.FormatSafe(fmt, arg...);
	}
}