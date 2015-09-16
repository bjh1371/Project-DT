////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file EchoServerMain.cpp
/// \author bjh1371
/// \date 2015/08/03
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "EchoServer.h"

#include "SharedPacketHandlerSetup.h"

namespace
{
	void ServerMain(HINSTANCE hInstance)
	{
		if (AppEnv::Setup())
		{
			SetupSharedPacketHandler();
			
			CEchoServer* EchoServer = nullptr;
			EchoServer = xnew(CEchoServer, hInstance, _T("EchoServer"));
	
			if (EchoServer)
			{
				EchoServer->Start();
			}
	
			Generic::SafeDelete(EchoServer);
			AppEnv::TearDown();
		}
	}
}


int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	// ���ڷ� ���� ��� �ָܼ�� ������ â��� �����ؼ� �������� �ؾߵ�
	if (lpCmdLine[0] == 1)
	{

	}
	else if (lpCmdLine[0] == 2)
	{

	}
	else
	{
		ServerMain(hInstance);
	}

	return 0;
}


