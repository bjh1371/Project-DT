////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file EchoClientListener.h
/// \author bjh1371
/// \date 2015/08/03
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Base/AsyncPeerListener.h"

class CAsyncTcpPeer;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \class CEchoClientListener
/// \brief 
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CEchoClientListener : public CAsyncPeerListener
{
public:
	/// \brief 积己磊
	CEchoClientListener(short port);
	
	/// \brief 家戈磊
	virtual ~CEchoClientListener();
	
	
public:
	/// \brief 乔绢 积己
	virtual CAsyncTcpPeer* CreatePeer() override;
};