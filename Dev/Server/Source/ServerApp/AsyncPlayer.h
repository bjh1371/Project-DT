////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file AsyncPlayer.h
/// \author bjh1371
/// \date 2015/09/07
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Base/AsyncTcpPeer.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \class CAsyncPlayer
/// \brief Player�� BaseŬ����
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CAsyncPlayer : public CAsyncTcpPeer
{
private:
	CAsyncTimerEvent* m_PingTimer;  ///< �� Ÿ�̸�


public:
	/// \brief ������
	CAsyncPlayer();
	
	/// \brief �Ҹ���
	virtual ~CAsyncPlayer();
	
	
public:
	/// \brief ���ӽ� ȣ��
	virtual void OnConnected() override;

	/// \brief ���� ����� ȣ��
	virtual void OnDisconnected() override;


private:
	/// \brief Ÿ�̸�
	void OnTimer(CAsyncTimerEvent* evt) override;
};