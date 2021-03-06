////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file BootStrap.h
/// \author bjh1371
/// \date 2014/09/20
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Function.h"

namespace BootStrap
{
	typedef std::function<void()>  CExitFunc;
	typedef std::vector<CExitFunc> CExitFuncArray;

	// 프로그램이 종료될때 실행할 함수를 등록한다.
	void AddExitFunction(const CExitFunc& exitFunc);

	// 모든 등록된 함수를 실행한다.
	void ExcuteAllFunction();
}