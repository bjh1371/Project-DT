////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file FileLog.cpp
/// \author bjh1371
/// \date 2015/07/07
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "FileLog.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief 생성자
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CFileLog::CFileLog(LPCTSTR fileName)
:
m_File(INVALID_HANDLE_VALUE)
{
	SafeGuard();

	if (fileName == nullptr)
	{
		TCHAR basicFilePath[MAX_PATH];
		GetModuleFileName(NULL, basicFilePath, MAX_PATH);

		LPTSTR dot = _tcsrchr(basicFilePath, _T('.'));
		*dot = 0;
		_tcscat_s(basicFilePath, MAX_PATH, _T(".log"));
		
		Open(basicFilePath);
	}
	else
	{
		Open(fileName);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief 소멸자
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CFileLog::~CFileLog()
{
	SafeGuard();

	Close();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief 
/// \param LPCTSTR fileName 
/// \return bool
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CFileLog::Open(LPCTSTR fileName)
{
	SafeGuard();

	bool success = false;

	// 기존에 파일이 열려있으면 닫아주고
	if (m_File != INVALID_HANDLE_VALUE)
	{
		Close();
	}

	m_File = CreateFile(fileName, GENERIC_WRITE, FILE_SHARE_READ, nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
	if (m_File != INVALID_HANDLE_VALUE)
	{
		// 파일이 없어서 새로 만들었다면
		if (GetLastError() != ERROR_ALREADY_EXISTS)
		{
	
#if defined(UNICODE)
			const unsigned short mark = 0xFEFF;
			DWORD written = 0;
			WriteFile(m_File, &mark, sizeof(mark), &written, nullptr);
#endif

		}
		success = true;
		SetFilePointer(m_File, 0, 0, FILE_END);
	}

	return success;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief 
/// \param LPCTSTR log 
/// \return void
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CFileLog::Write(LPCTSTR log)
{
	SafeGuard();

	if (m_File != INVALID_HANDLE_VALUE)
	{
		DWORD written = 0;
		size_t len = _tcslen(log) * sizeof(TCHAR);
		
		WriteFile(m_File, log, static_cast<DWORD>(len), &written, nullptr);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief 
/// \return void
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CFileLog::Close()
{
	SafeGuard();

	if (m_File != INVALID_HANDLE_VALUE)
	{
		FlushFileBuffers(m_File);

		CloseHandle(m_File);
		m_File = INVALID_HANDLE_VALUE;
	}
}
