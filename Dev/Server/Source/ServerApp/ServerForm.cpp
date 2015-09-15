////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file ServerForm.cpp
/// \author bjh1371
/// \date 2015/07/03
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <commctrl.h>
#include <Richedit.h>

#include "ServerForm.h"

#include "Base/Lifespan.h"
#include "Base/ThreadRegistry.h"
#include "Base/Log.h"
#include "Base/Function.h"
#include "Base/PathString.h"

#pragma comment(lib, "comctl32")

namespace
{
	/// 서버 메인 창 너비
	const int WIDTH = 800;

	/// 서버 메인 창 높이
	const int HEIGHT = 600;

	/// 리스트 높이 비율
	const int LIST_HEIGHT = 1;

	/// 에디트 높이 비율
	const int EDIT_HEIGHT = 2;

	/// \brief 컬럼 키 너비 비율
	const int KEY_WIDTH = 1;
	
	/// \brief 컬럼 값 너비 비율
	const int VALUE_WIDTH = 3;

	/// 스크롤 마진
	const int EDIT_SCROLL_MARGIN = 9;

	/// 리스트 마진
	const int LIST_SCROLL_MARGINE = 6;

	/// \brief 컨트롤들 높이 구하기
	int GetHeight(int height, int control)
	{
		return height / (LIST_HEIGHT + EDIT_HEIGHT) * control;
	}

	/// \brief 너비 구하기
	int GetWidth(int width)
	{
		return width - EDIT_SCROLL_MARGIN;
	}

	/// \brief 컬럼 너비 구하기
	int GetColumnWidth(int width)
	{
		return (width - LIST_SCROLL_MARGINE * 2) / (KEY_WIDTH + VALUE_WIDTH);
	}

	/// \brief 컬럼 속성을 셋팅한다.
	void SetColumnAttribute(LV_COLUMN& column, LPTSTR name, int width, int rate)
	{
		width = GetColumnWidth(width);
		column.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
		column.fmt = LVCFMT_LEFT;
		column.pszText = name;
		column.iSubItem = 0;
		column.cx = width * rate;
	}

	/// \brief 리스트 아이템 속성을 셋팅한다.
	void SetListItemAttivute(LV_ITEM& item, LPTSTR name, int index)
	{
		item.mask = LVIF_TEXT;
		item.iItem = index;
		item.iSubItem = 0;
		item.pszText = name;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief 생성자
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CServerForm::CServerForm(HINSTANCE hInstance, LPCTSTR title, LPCTSTR guid)
:
m_Mutex(NULL),
m_RichEditDll(LoadLibrary(_T("msftedit.dll"))),
m_MainWindow(NULL),
m_ListView(NULL),
m_Edit(NULL),
m_Instance(hInstance),
m_Title(title),
m_Good(false),
m_Lock(),
m_ListItemArray(),
m_LogQueue()
{
	SafeGuard();
	
	if (hInstance != NULL)
	{
		m_Mutex = CreateMutex(NULL, true, guid);
		
		// 중복 실행되고 있나?
		if (GetLastError() != ERROR_ALREADY_EXISTS)
		{
			m_Good = true;
		}
	}
	
	// 정상적이면 ServerForm 쓰레드 시작
	if (m_Good)
	{
		Start();
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief 소멸자
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CServerForm::~CServerForm()
{
	SafeGuard();

	if (m_RichEditDll)
	{
		FreeLibrary(m_RichEditDll);
	}

	Stop();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief 강제로 정지 시킨다.
/// \return void
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CServerForm::Stop()
{
	SendMessage(m_MainWindow, WM_DESTROY, NULL, NULL);

	// 쓰레드 끝날때까지 기다림
	Join();

	if (m_Mutex != NULL)
	{
		ReleaseMutex(m_Mutex);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief 아이템을 포스트한다.
/// \param const tstring & key 키 
/// \param const tstring & value 값
/// \return void
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CServerForm::PostItem(LPCTSTR key, int value)
{
	SafeGuard();

	// 추가 해야되는가?
	bool add = true;

	// 포스트 해야되는가?
	bool isPost = true;

	m_Lock.SynchronizeForRead([&]
	{
		for (int i = 0; i < m_ListItemArray.size(); ++i)
		{
			CListItem& item = m_ListItemArray[i];
			if (_tcscmp(key, item.Key) == 0)
			{
				// 존재하는 아이템
				add = false;

				// 값이 바뀌었으면 수정
				if (item.ValueCache != value)
				{
					_stprintf_s(item.Value, ARRAYSIZE(item.Value), _T("%d"), value);
					item.ValueCache = value;
					item.RecentEvent = LIST_EVENT_MODIFY;
				}
				// 바뀌지 않았다면 포스트하지 않음
				else
				{
					isPost = false;
				}

				break;
			}
		}
	});

	// 아이템 추가
	if (add)
	{
		CListItem item;
		_tcsncpy_s(item.Key, ARRAYSIZE(item.Key), key, _TRUNCATE);
		_stprintf_s(item.Value, ARRAYSIZE(item.Value), _T("%d"), value);
		item.ValueCache = value;
		item.RecentEvent = LIST_EVENT_ADD;

		// 컨테이너에 추가
		m_Lock.SynchronizeForWrite([&]{
			m_ListItemArray.push_back(item);
		});
	}

	if (isPost)
	{
		SendMessage(m_MainWindow, WM_POST_LIST, NULL, NULL);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief 아이템을 포스트한다.
/// \param LPCTSTR key 키
/// \param LPCTSTR value 값
/// \return void
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CServerForm::PostItem(LPCTSTR key, LPCTSTR value)
{
	SafeGuard();

	// 추가 해야되는가?
	bool add = true;

	// 포스트 해야되는가?
	bool isPost = true;

	m_Lock.SynchronizeForRead([&]
	{
		for (int i = 0; i < m_ListItemArray.size(); ++i)
		{
			CListItem& item = m_ListItemArray[i];
			if (_tcscmp(key, item.Key) == 0)
			{
				// 존재하는 아이템
				add = false;

				_tcsncpy_s(item.Value, ARRAYSIZE(item.Value), value, _TRUNCATE);
				item.RecentEvent = LIST_EVENT_MODIFY;

				break;
			}
		}
	});

	// 아이템 추가
	if (add)
	{
		CListItem item;
		_tcsncpy_s(item.Key, ARRAYSIZE(item.Key), key, _TRUNCATE);
		_tcsncpy_s(item.Value, ARRAYSIZE(item.Value), value, _TRUNCATE);
		item.ValueCache = 0;
		item.RecentEvent = LIST_EVENT_ADD;

		// 컨테이너에 추가
		m_Lock.SynchronizeForWrite([&]{
			m_ListItemArray.push_back(item);
		});
	}

	if (isPost)
	{
		SendMessage(m_MainWindow, WM_POST_LIST, NULL, NULL);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief 
/// \param LPCTSTR file 
/// \param LPCTSTR func 
/// \param int line 
/// \param LPCTSTR timeStamp 
/// \param LPCTSTR context 
/// \return void
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CServerForm::PostLog(LPCTSTR file, LPCTSTR func, int line, LPCTSTR timeStamp, LPCTSTR context)
{
	SafeGuard();

	CFixedString<2048> log;
	m_LogQueue.Add(log.FormatSafe(_T("%s | %s \r\n"), timeStamp, context));

	SendMessage(m_MainWindow, WM_POST_EDIT, NULL, NULL);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief 
/// \param int width 
/// \param int height 
/// \return void
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CServerForm::OnResize(int width, int height)
{
	SafeGuard();

	MoveWindow(m_ListView, 0, 0, GetWidth(width), GetHeight(height, LIST_HEIGHT), TRUE);
	MoveWindow(m_Edit, 0, GetHeight(height, LIST_HEIGHT), GetWidth(width), GetHeight(height, EDIT_HEIGHT), TRUE);

	LV_COLUMN key;
	SetColumnAttribute(key, _T("key"), width, KEY_WIDTH);

	LV_COLUMN value;
	SetColumnAttribute(value, _T("value"), width, VALUE_WIDTH);

	ListView_SetColumn(m_ListView, 0, &key);
	ListView_SetColumn(m_ListView, 1, &value);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief 
/// \return void
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CServerForm::OnPostList()
{
    SafeGuard();

	for (int i = 0; i < m_ListItemArray.size(); ++i)
	{
		CListItem& item = m_ListItemArray[i];

		// 추가 
		if (item.RecentEvent == LIST_EVENT_ADD)
		{
			LV_ITEM lvItem;
			SetListItemAttivute(lvItem, item.Key, i);
			ListView_InsertItem(m_ListView, &lvItem);
		}

		// 추가 / 수정 이벤트일 경우 리스트 값도 셋팅
		if (item.RecentEvent == LIST_EVENT_ADD || item.RecentEvent == LIST_EVENT_MODIFY)
		{
			ListView_SetItemText(m_ListView, i, 1, item.Value);
			item.RecentEvent = LIST_EVENT_NONE;
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief 
/// \return void
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CServerForm::OnPostEdit()
{
	SafeGuard();

	m_LogQueue.AcceptPendingItem();
	for (auto& log : m_LogQueue)
	{
		LRESULT lines = SendMessage(m_Edit, EM_GETLINECOUNT, 0, 0);
		LRESULT chars = SendMessage(m_Edit, WM_GETTEXTLENGTH, 0, 0);
		SendMessage(m_Edit, EM_SETSEL, chars, chars);
		if (m_RichEditDll)
		{
			CHARFORMAT cf;
			memset(&cf, 0, sizeof(cf));
			cf.cbSize = sizeof cf;
			cf.dwMask = CFM_COLOR;

			switch (log[0])
			{
			case _T('D'): cf.crTextColor = RGB(0x8A, 0x2B, 0xE2); break; // debug
				//case _T('S'): cf.crTextColor = RGB(0x22, 0x8B, 0x22); break; // status
			case _T('W'): cf.crTextColor = RGB(0xB8, 0x86, 0x0B); break; // warning
			case _T('E'): cf.crTextColor = RGB(0xFF, 0x00, 0x00); break; // error
			default:      cf.crTextColor = RGB(0x00, 0x00, 0x00); break; // warning
			}

			SendMessage(m_Edit, EM_SETCHARFORMAT, SCF_SELECTION, reinterpret_cast<LPARAM>(&cf));
		}

		SendMessage(m_Edit, EM_REPLACESEL, TRUE, reinterpret_cast<LPARAM>(log.c_str()));
		SendMessage(m_Edit, EM_LINESCROLL, 0, SendMessage(m_Edit, EM_GETLINECOUNT, 0, 0) - lines);
		// SendMessage(m_Edit, EM_REPLACESEL, 0, (LPARAM)log.c_str());
	}
	m_LogQueue.clear();
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief 
/// \param int command 
/// \return void
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CServerForm::OnCommand(int command)
{
	switch (command)
	{
    // Edit Window TextBuffer is Full
	case EN_MAXTEXT:
		SetWindowText(m_Edit, _T(""));
		break;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief 
/// \param HWND hWnd 
/// \param UINT iMessage 
/// \param WPARAM wParam 
/// \param LPARAM lParam 
/// \return LRESULT CALLBACK
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
LRESULT CALLBACK CServerForm::Proc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	CServerForm* serverForm = nullptr;
	
	switch (iMessage)
	{
	case WM_CREATE:
		SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(((LPCREATESTRUCT)lParam)->lpCreateParams));
		break;

	case WM_POST_LIST:
		serverForm = reinterpret_cast<CServerForm*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
		serverForm->OnPostList();
		break;

	case WM_POST_EDIT:
		serverForm = reinterpret_cast<CServerForm*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
		serverForm->OnPostEdit();
		break;

	case WM_SIZE:
		serverForm = reinterpret_cast<CServerForm*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
		serverForm->OnResize(LOWORD(lParam), HIWORD(lParam));
		break;

	case WM_COMMAND:
		serverForm = reinterpret_cast<CServerForm*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
		serverForm->OnCommand(HIWORD(wParam));
		break;
	case WM_DESTROY:
		// 메인 스레드 루프 종료
		TerminateApp();

		PostQuitMessage(0);
		return 0;
	}

	return(DefWindowProc(hWnd, iMessage, wParam, lParam));
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief 
/// \return void
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CServerForm::ThreadMain()
{
	SafeGuard();

	// 로그 싱크 등록
	Log::AddSink(FuncUtil::Bind(this, &CServerForm::PostLog));

	WNDCLASS WndClass;

	WndClass.cbClsExtra = 0;
	WndClass.cbWndExtra = 0;
	WndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	WndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	WndClass.hInstance = m_Instance;
	WndClass.lpfnWndProc = (WNDPROC)Proc;
	WndClass.lpszClassName = m_Title;
	WndClass.lpszMenuName = NULL;
	WndClass.style = CS_HREDRAW | CS_VREDRAW;
	
	RegisterClass(&WndClass);

	CFixedString<128> caption;
	caption.FormatSafe(_T("%a - %a"), m_Title, CPathString::GetCurrentFullDir());
	
	m_MainWindow = CreateWindow(
		m_Title, caption.Get(), WS_OVERLAPPEDWINDOW,
		100, 100, WIDTH, HEIGHT,
		NULL, (HMENU)NULL, m_Instance, this
		);

	m_ListView = CreateWindowEx(0, WC_LISTVIEWW, NULL,
		WS_VISIBLE | WS_CHILD | WS_BORDER | LVS_REPORT,
		0, 0, GetWidth(WIDTH), GetHeight(HEIGHT, LIST_HEIGHT), m_MainWindow, NULL, m_Instance, NULL);

	if (m_RichEditDll)
	{
		m_Edit = CreateWindowEx(0, MSFTEDIT_CLASS, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS |
			WS_BORDER | WS_VSCROLL | WS_HSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL | ES_READONLY,
			0, GetHeight(HEIGHT, LIST_HEIGHT), GetWidth(WIDTH), GetHeight(HEIGHT, EDIT_HEIGHT),
			m_MainWindow, NULL, m_Instance, NULL);

		SendMessage(m_Edit, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), TRUE);
	}
 	else
	{
		m_Edit = CreateWindow(_T("Edit"), NULL,
		WS_CHILD | WS_VISIBLE | WS_BORDER | WS_HSCROLL | WS_VSCROLL |
		ES_MULTILINE | ES_AUTOHSCROLL | ES_AUTOVSCROLL | ES_READONLY,
		0, GetHeight(HEIGHT, LIST_HEIGHT), GetWidth(WIDTH), GetHeight(HEIGHT, EDIT_HEIGHT), m_MainWindow, NULL, m_Instance, NULL);
	}

	ListView_SetExtendedListViewStyle(m_ListView, LVS_EX_GRIDLINES);

	ShowWindow(m_MainWindow, SW_SHOWNORMAL);

	LV_COLUMN key;
	SetColumnAttribute(key, _T("key"), WIDTH, KEY_WIDTH);
	
	LV_COLUMN value;
	SetColumnAttribute(value, _T("value"), WIDTH, VALUE_WIDTH);

	ListView_InsertColumn(m_ListView, 0, &key);
	ListView_InsertColumn(m_ListView, 1, &value);

	MSG message;
	while (GetMessage(&message, 0, 0, 0)) {
		TranslateMessage(&message);
		DispatchMessage(&message);
	}
}