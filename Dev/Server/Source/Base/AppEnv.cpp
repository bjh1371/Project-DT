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
	// ó�� �����ϴ� ȯ�� ���� ����
	bool Setup()
	{
		bool success = false;

		// ���� ������ �ε��� �Ҵ�
		AllocateThreadIndex();

		// �̴ϴ��� ���
		SetUnhandledExceptionFilter(ExceptionFilter);

		// �ð� ����
		Clock::Setup();

		do 
		{
			// ���� �ڿ� �¾�
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
			
			// ������� ���� ����
			success = true;

		} while (0);

		return success;
	}

	// ������ ȯ������ �ڿ� �����Ұ� ������ ����
	void TearDown()
	{
		// ���� �ڿ� ����
		CAsyncTcpSocket::Teardown();

		// ����� �Լ��� ����
		BootStrap::ExcuteAllFunction();

		// �޸� �� ����Ʈ
		Memory::ReportMemoryLeak();
	}
}