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
	/// ������ �̺�Ʈ
	enum
	{
		WM_POST_LIST = WM_USER + 1, ///< ����Ʈ �̺�Ʈ
		WM_POST_EDIT = WM_USER + 2, ///< ����Ʈ �̺�Ʈ

		MAX_ITEM_LENGTH = 64,
	};

	/// ����Ʈ ������ �̺�Ʈ
	enum ListEvent
	{
		LIST_EVENT_NONE,
		LIST_EVENT_ADD,
		LIST_EVENT_MODIFY,
	};

	/// \brief ����Ʈ ������
	struct CListItem
	{
		TCHAR     Key[MAX_ITEM_LENGTH];   ///< Ű
		TCHAR     Value[MAX_ITEM_LENGTH]; ///< ��
		int       ValueCache;             ///< �� ��Ʈ
		ListEvent RecentEvent;            ///< �ֱ� �Ͼ �̺�Ʈ

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
	HANDLE            m_Mutex;         ///< �ߺ� ���� ������ ���ؽ�
	HWND              m_MainWindow;    ///< ������ �ڵ�
	HWND              m_ListView;      ///< ����Ʈ�� �ڵ�
	HWND              m_Edit;          ///< ����Ʈ �ڵ�
	HINSTANCE         m_Instance;      ///< �ν��Ͻ�
	LPCTSTR           m_Title;         ///< Ÿ��Ʋ
	bool              m_Good;          ///< ���������� �ʱ�ȭ ��
	CReaderWriterLock m_Lock;          ///< ����Ʈ ������ ��� ��ȣ ��
	CListItemArray    m_ListItemArray; ///< ����Ʈ ������ ���
	CLogQueue         m_LogQueue;      ///< �α� ť
	

public:
	/// \brief ������
	CServerForm(HINSTANCE hInstance, LPCTSTR title, LPCTSTR guid);
	
	/// \brief �Ҹ���
	virtual ~CServerForm();
	
	
public:
	/// \brief ����
	virtual void Stop();

	/// \brief ���������� �ʱ�ȭ �Ǿ���?
	bool IsGood() const { return m_Good; }


public:
	/// \brief �������� ����Ʈ �Ѵ�.
	void PostItem(LPCTSTR key, int value);

	/// \brief �������� ����Ʈ �Ѵ�.
	void PostItem(LPCTSTR key, LPCTSTR value);

	/// \brief �α׸� ����Ʈ�Ѵ�.
	void PostLog(LPCTSTR file, LPCTSTR func, int line, LPCTSTR timeStamp, LPCTSTR context);


private:
	/// \brief ������ ����
	void OnResize(int width, int height);

	/// \brief ������ ����Ʈ
	void OnPostList();

	/// \brief ������ ����Ʈ
	void OnPostEdit();

	/// \brief Command �̺�Ʈ
	void OnCommand(int command);
	

private:
	/// \brief ���ν���
	static LRESULT CALLBACK Proc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);


private:
	/// \brief �۾� �Լ�
	void ThreadMain();
};