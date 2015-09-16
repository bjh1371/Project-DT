////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file Clock.h
/// \author 
/// \date 2014.11.5
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

typedef ULONGLONG Milli_t;

namespace Clock
{
	/// \brief ���� �ð�
	extern Milli_t g_StartTime;

	/// \brief ���Ѵ� �ð�
	const Milli_t BOUNDLESS = 0xFFFFFFFFFFFFFFFF;

	/// \brief �ð� �¾�
	void Setup();
	
	/// \brief ���� �ð� �и��ʷ� ��ȯ
	inline Milli_t GetCurrentMilli()
	{
		return GetTickCount64() - g_StartTime;
	}

	/// \brief ���� �ð��� �־��� �ð��� ���ؼ� ��ȯ�Ѵ�.
	inline Milli_t GetElaspedMilli(Milli_t t)
	{
		return GetCurrentMilli() + t;
	}

	/// \brief ������ �ð��� �����ΰ�?
	inline bool IsPast(Milli_t t)
	{
		return GetCurrentMilli() > t;
	}
}