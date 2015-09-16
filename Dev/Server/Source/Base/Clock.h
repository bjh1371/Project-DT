////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file Clock.h
/// \author 
/// \date 2014.11.5
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

typedef ULONGLONG Milli_t;

namespace Clock
{
	/// \brief 시작 시간
	extern Milli_t g_StartTime;

	/// \brief 무한대 시간
	const Milli_t BOUNDLESS = 0xFFFFFFFFFFFFFFFF;

	/// \brief 시간 셋업
	void Setup();
	
	/// \brief 현재 시간 밀리초로 반환
	inline Milli_t GetCurrentMilli()
	{
		return GetTickCount64() - g_StartTime;
	}

	/// \brief 현재 시간에 주어진 시간을 더해서 반환한다.
	inline Milli_t GetElaspedMilli(Milli_t t)
	{
		return GetCurrentMilli() + t;
	}

	/// \brief 지정된 시간이 과거인가?
	inline bool IsPast(Milli_t t)
	{
		return GetCurrentMilli() > t;
	}
}