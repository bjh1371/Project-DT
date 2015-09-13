////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file AppEnv.cpp
/// \author bjh1371
/// \date 2015/07/03
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "AppEnv.h"

#include "AsyncTcpSocket.h"
#include "Exception.h"
#include "Lifespan.h"
#include "PathString.h"
#include "ThreadRegistry.h"
#include "BootStrap.h"

namespace AppEnv
{
	// 처음 시작하는 환경 정보 셋팅
	bool Setup()
	{
		bool success = false;

		// 메인 스레드 인덱스 할당
		AllocateThreadIndex();

		// 미니덤프 등록
		SetUnhandledExceptionFilter(ExceptionFilter);

		// 시간 시작
		Clock::Setup();

		do 
		{
			// 소켓 자원 셋업
			if (CAsyncTcpSocket::Setup() == false)
			{
				TRACE(_T("WsaStartup Error"));
				break;
			}

			if (SetCurrentDirectory(CPathString::GetCurrentDir()) == FALSE)
			{
				TRACE(_T("SetCurrentDirectory Error"));
				break;
			}
			
			// 여기까지 오면 성공
			success = true;

		} while (0);

		return success;
	}

	// 셋팅한 환경정보 자원 해제할거 있으면 해제
	void TearDown()
	{
		// 소켓 자원 해제
		CAsyncTcpSocket::Teardown();

		// 예약된 함수들 실행
		BootStrap::ExcuteAllFunction();

		// 메모리 릭 리포트
		Memory::ReportMemoryLeak();
	}
}