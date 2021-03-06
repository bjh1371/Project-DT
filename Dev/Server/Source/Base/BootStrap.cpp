////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file BootStrap.cpp
/// \author bjh1371
/// \date 2014/09/20
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "BootStrap.h"

namespace BootStrap
{
	CExitFuncArray g_ExitFuncArray;

	// 함수 종료직전 실행할 함수를 추가한다.
	void AddExitFunction(const CExitFunc& exitFunc)
	{
		g_ExitFuncArray.push_back(exitFunc);
	}

	// 모든 함수를 실행한다.
	void ExcuteAllFunction()
	{
		for (auto& func : g_ExitFuncArray)
		{
			func();
		}
	}
}