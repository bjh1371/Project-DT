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
	// �⺻���� ���Ϸΰ�
	void FileLogger(LPCTSTR file, LPCTSTR func, int line, LPCTSTR timeStamp, LPCTSTR context)
	{
		CFileLog fileLog;
		fileLog.WriteFormatted(_T("%a | %a \r\n"), timeStamp, context);

	}

	std::vector<Sink> g_SinkArray = { FuncUtil::Bind(FileLogger), };

	///< �α� ��ũ �߰�
	void AddSink(Sink sink)
	{
		g_SinkArray.push_back(sink);
	}

	/// \brief �α� �߻�
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