////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file Log.cpp
/// \author 
/// \date 2014.10.24
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "FileLog.h"
#include "Function.h"
#include "Log.h"

namespace Log
{
	// 기본적인 파일로거
	void FileLogger(LPCTSTR file, LPCTSTR func, int line, LPCTSTR timeStamp, LPCTSTR context)
	{
		CFileLog fileLog;
		fileLog.WriteFormatted(_T("%a | %a \r\n"), timeStamp, context);

	}

	std::vector<Sink> g_SinkArray = { FuncUtil::Bind(FileLogger), };

	///< 로그 싱크 추가
	void AddSink(Sink sink)
	{
		g_SinkArray.push_back(sink);
	}

	/// \brief 로그 발생
	void EmitInternal(LPCTSTR file, LPCTSTR func, int line, LPCTSTR context)
	{
		CFixedString<256> timeStamp;
		timeStamp.MakeDateTime();
		for (auto& sink : g_SinkArray)
		{
			sink(file, func, line, timeStamp, context);
		}
	}
}