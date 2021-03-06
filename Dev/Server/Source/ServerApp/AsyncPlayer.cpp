////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file AsyncPlayer.cpp
/// \author bjh1371
/// \date 2015/09/07
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "AsyncPlayer.h"

#include "PacketProc/PacketMarshaller.h"
#include "PacketProc/PacketBuffer.h"

#include "Packet/SharedPacket.h"

#include "Base/AsyncTimerRegistry.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief 생성자
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CAsyncPlayer::CAsyncPlayer()
:
CAsyncTcpPeer(xnew(CPacketMarshaller, PACKET_GROUP_SHARED))
{
	SafeGuard();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief 소멸자
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CAsyncPlayer::~CAsyncPlayer()
{
	SafeGuard();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief 
/// \return void
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAsyncPlayer::OnConnected()
{
	SafeGuard();

	__super::OnConnected();

	// 핑 타이머 시작
	// m_PingTimer = g_AsyncTimerRegistry->AddTimer(this, 10 * 1000, Clock::BOUNDLESS);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief 
/// \return void
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAsyncPlayer::OnDisconnected()
{
	SafeGuard();

	__super::OnDisconnected();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief 
/// \param CAsyncTimerEvent * evt 
/// \return void
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAsyncPlayer::OnTimer(CAsyncTimerEvent* evt)
{
	SafeGuard();

	if (m_PingTimer == evt)
	{
		CPacketWriter<PKTEcho> echoPacket;
		echoPacket->SetStr(_T("Ping"));
		PostSend(echoPacket);

		TRACE(_T("Ping send"));
	}
}