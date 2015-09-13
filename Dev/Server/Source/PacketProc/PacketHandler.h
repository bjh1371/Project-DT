////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file PacketHandler.h
/// \author 
/// \date 2014.10.24
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Packet/PacketBase.h"
#include "Packet/SharedPacket.h"
#include "Packet/ServerPacket.h"

#include "PacketBuffer.h"

class CAsyncTcpPeer;

class CClientPeer;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \class CPacketHandler
/// \brief 
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CPacketHandler
{
private:

public:
	/// \brief ������
	CPacketHandler();

	/// \brief �Ҹ���
	virtual ~CPacketHandler();


public:
	/// \brief ��Ŷ ID
	virtual PacketId_t GetPacketId() = 0;

	/// \brief ����
	virtual int Execute(CAsyncTcpPeer* peer, char* buf, int len);

	/// \brief ����
	virtual int Execute(CClientPeer* peer, char* buf, int len);
};


#define DECLARE_PACKET_HANDLER_INTERFACE(CLASS, PACKETID) \
	public:\
	CLASS()  = default; \
	~CLASS() = default; \
	virtual PacketId_t GetPacketId() override { return PACKETID; } \
	virtual int Execute(CSession* session, char* buf, int len) override; 

