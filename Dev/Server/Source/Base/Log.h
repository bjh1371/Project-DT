////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file Log.h
/// \author 
/// \date 2014.10.24
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "FixedString.h"
#include "Function.h"

namespace Log
{
	typedef CFunction<LPCTSTR, LPCTSTR, int, LPCTSTR, LPCTSTR> Sink;

	// 싱크 추가
	void AddSink(Sink sink);

	// 로그 발생
	void EmitInternal(LPCTSTR file, LPCTSTR func, int line, LPCTSTR context);

	// 로그 발생
	template <typename ...Arguments>
	void Emit(LPCTSTR file, LPCTSTR func, int line, LPCTSTR fmt, const Arguments&... args)
	{
		CFixedString<2048> log;
		log.FormatSafe(fmt, args...);

	    EmitInternal(file, func, line, log);
	}
}

#define TRACE(...) \
	Log::Emit(_T(__FILE__), _T(__FUNCTION__), __LINE__, __VA_ARGS__)