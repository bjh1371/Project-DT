////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file ServerApp.h
/// \author bjh1371
/// \date 2015/07/03
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Base/AsyncTcpEventSink.h"

class CServerForm;
class CAsyncTimerEvent;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \class CServerApp
/// \brief 
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CServerApp : public CAsyncEventSink
{
private:
	CServerForm*      m_ServerForm;  ///< ������ ��
	CAsyncTimerEvent* m_StatusTimer; ///< Ÿ�̸� �̺�Ʈ


public:
	/// \brief ������
	CServerApp(HINSTANCE hInstance, LPCTSTR title, LPCTSTR guid);
	
	/// \brief �Ҹ���
	virtual ~CServerApp();
	
	
public:
	/// \brief ����
	void Start();


public:
	/// \brief �������� ����Ʈ�Ѵ�.
	void PostItem(LPCTSTR key, int value);

	/// \brief �������� ����Ʈ�Ѵ�.
	void PostItem(LPCTSTR key, LPCTSTR value);


protected:
	/// \brief ���� ����
	virtual void ReportStatus();

	/// \brief ���� ��� Ÿ�̸�
	virtual void OnTimer(CAsyncTimerEvent* evt) override;


private:
	/// \brief �ڿ� ����
	virtual bool Setup() = 0;

	/// \brief �ڿ� ����
	virtual void TearDown() = 0;
};