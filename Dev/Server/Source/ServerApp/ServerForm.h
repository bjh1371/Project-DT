////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file ServerForm.h
/// \author bjh1371
/// \date 2015/07/03
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Base/ThreadSafeQueue.h"
#include "Base/Thread.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \class CServerForm
/// \brief 
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CServerForm : public CThread
{
private:
	/// 윈도우 이벤트
	enum
	{
		WM_POST_LIST = WM_USER + 1, ///< 리스트 이벤트
		WM_POST_EDIT = WM_USER + 2, ///< 에디트 이벤트

		MAX_ITEM_LENGTH = 64,
	};

	/// 리스트 아이템 이벤트
	enum ListEvent
	{
		LIST_EVENT_NONE,
		LIST_EVENT_ADD,
		LIST_EVENT_MODIFY,
	};

	/// \brief 리스트 아이템
	struct CListItem
	{
		TCHAR     Key[MAX_ITEM_LENGTH];   ///< 키
		TCHAR     Value[MAX_ITEM_LENGTH]; ///< 값
		int       ValueCache;             ///< 값 인트
		ListEvent RecentEvent;            ///< 최근 일어난 이벤트

		CListItem()
		:
		ValueCache(0),
		RecentEvent(LIST_EVENT_NONE)
		{
			Key[0] = 0;
			Value[0] = 0;
		}
	};


private:
	typedef stx::vector<CListItem> CListItemArray;
	typedef CThreadSafeQueue<tstring> CLogQueue;
	

private:
	HANDLE            m_Mutex;         ///< 중복 실행 방지용 뮤텍스
	HWND              m_MainWindow;    ///< 윈도우 핸들
	HWND              m_ListView;      ///< 리스트뷰 핸들
	HWND              m_Edit;          ///< 에디트 핸들
	HINSTANCE         m_Instance;      ///< 인스턴스
	LPCTSTR           m_Title;         ///< 타이틀
	bool              m_Good;          ///< 정상적으로 초기화 됨
	CReaderWriterLock m_Lock;          ///< 리스트 아이템 목록 보호 락
	CListItemArray    m_ListItemArray; ///< 리스트 아이템 목록
	CLogQueue         m_LogQueue;      ///< 로그 큐
	

public:
	/// \brief 생성자
	CServerForm(HINSTANCE hInstance, LPCTSTR title, LPCTSTR guid);
	
	/// \brief 소멸자
	virtual ~CServerForm();
	
	
public:
	/// \brief 시작
	virtual void Stop();

	/// \brief 정상적으로 초기화 되었나?
	bool IsGood() const { return m_Good; }


public:
	/// \brief 아이템을 포스트 한다.
	void PostItem(LPCTSTR key, int value);

	/// \brief 아이템을 포스트 한다.
	void PostItem(LPCTSTR key, LPCTSTR value);

	/// \brief 로그를 포스트한다.
	void PostLog(LPCTSTR file, LPCTSTR func, int line, LPCTSTR timeStamp, LPCTSTR context);


private:
	/// \brief 사이즈 조절
	void OnResize(int width, int height);

	/// \brief 아이템 포스트
	void OnPostList();

	/// \brief 아이템 포스트
	void OnPostEdit();

	/// \brief Command 이벤트
	void OnCommand(int command);
	

private:
	/// \brief 프로시저
	static LRESULT CALLBACK Proc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);


private:
	/// \brief 작업 함수
	void ThreadMain();
};