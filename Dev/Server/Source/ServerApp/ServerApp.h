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
	CServerForm*      m_ServerForm;  ///< 윈도우 폼
	CAsyncTimerEvent* m_StatusTimer; ///< 타이머 이벤트


public:
	/// \brief 생성자
	CServerApp(HINSTANCE hInstance, LPCTSTR title, LPCTSTR guid);
	
	/// \brief 소멸자
	virtual ~CServerApp();
	
	
public:
	/// \brief 시작
	void Start();


public:
	/// \brief 아이템을 포스트한다.
	void PostItem(LPCTSTR key, int value);

	/// \brief 아이템을 포스트한다.
	void PostItem(LPCTSTR key, LPCTSTR value);


protected:
	/// \brief 상태 보고
	virtual void ReportStatus();

	/// \brief 상태 출력 타이머
	virtual void OnTimer(CAsyncTimerEvent* evt) override;


private:
	/// \brief 자원 셋팅
	virtual bool Setup() = 0;

	/// \brief 자원 해제
	virtual void TearDown() = 0;
};