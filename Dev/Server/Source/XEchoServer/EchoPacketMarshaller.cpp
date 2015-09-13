////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file EchoPacketMarshaller.cpp
/// \author bjh1371
/// \date 2015/09/09
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "EchoPacketMarshaller.h"

#include "Base/AsyncTcpEvent.h"
#include "Base/AsyncTcpPeer.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief 생성자
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CEchoPacketMarshaller::CEchoPacketMarshaller()
{
	SafeGuard();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief 소멸자
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CEchoPacketMarshaller::~CEchoPacketMarshaller()
{
	SafeGuard();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief 
/// \param CAsyncTcpPeer * peer 
/// \param char * buf 
/// \param int total 
/// \return int
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CEchoPacketMarshaller::UnMarshall(CAsyncTcpPeer* peer, char* buf, int total)
{
	SafeGuard();

	int handled = 0;

	do
	{
		// 헤더 넘게 왔나?
		if (total < 4)
			break;

		// 사이즈
		int size = *(int*)buf;

		// 패킷 사이즈 만큼 왔나?
		if (total < size)
			break;

		handled = size;

		CAsyncTcpEvent* evt = CAsyncTcpEvent::Alloc(size);
		evt->IncreaseRefCount();
		evt->m_TotalBytes = size;
		memcpy(evt->m_Buf, buf, size);

		peer->PostSend(evt);

		if (evt->DecreaseRefCount() == 0)
		{
			CAsyncTcpEvent::Dealloc(evt);
		}

	} while (0);

	return handled;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief 
/// \param char * buf 
/// \param int len 
/// \return void
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CEchoPacketMarshaller::Marshall(char* buf, int len)
{
	SafeGuard();
}
