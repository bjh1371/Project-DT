////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file EchoServer.cpp
/// \author bjh1371
/// \date 2015/08/03
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "EchoServer.h"

#include "EchoClientListener.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief 持失切
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CEchoServer::CEchoServer(HINSTANCE hInstance,  LPCTSTR title)
:
CServerApp(hInstance, title, _T("guid"))
{
	SafeGuard();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief 社瑚切
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CEchoServer::~CEchoServer()
{
	SafeGuard();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief 
/// \return bool
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CEchoServer::Setup()
{
	SafeGuard();

	bool result = true;

	m_EchoClientListener = xnew(CEchoClientListener, 6010);
	m_EchoClientListener->Init(1000);

	return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief 
/// \return void
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CEchoServer::TearDown()
{
	SafeGuard();

	xdelete(m_EchoClientListener);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief 
/// \return void
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CEchoServer::ReportStatus()
{
	__super::ReportStatus();

	PostItem(_T("ClientPeer"), Generic::FormatSafe(_T("%a / %a"), m_EchoClientListener->GetFreeCount(), m_EchoClientListener->GetAllocCount()));
}
