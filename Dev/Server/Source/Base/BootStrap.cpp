////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file BootStrap.cpp
/// \author bjh1371
/// \date 2014/09/20
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "BootStrap.h"

namespace BootStrap
{
	CExitFuncArray g_ExitFuncArray;

	// �Լ� �������� ������ �Լ��� �߰��Ѵ�.
	void AddExitFunction(const CExitFunc& exitFunc)
	{
		g_ExitFuncArray.push_back(exitFunc);
	}

	// ��� �Լ��� �����Ѵ�.
	void ExcuteAllFunction()
	{
		for (auto& func : g_ExitFuncArray)
		{
			func();
		}
	}
}