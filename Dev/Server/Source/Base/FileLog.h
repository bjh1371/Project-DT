////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file FileLog.h
/// \author bjh1371
/// \date 2015/07/07
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \class CFileLog
/// \brief ���Ͽ� �α׸� �ۼ��ϱ� ���� Ŭ����
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CFileLog
{
private:
	HANDLE m_File; ///< ���� �ڵ�
	

public:
	/// \brief ������
	CFileLog(LPCTSTR fileName = nullptr);
	
	/// \brief �Ҹ���
	~CFileLog();


public:
	/// \brief ������ ����.
	bool Open(LPCTSTR fileName);

	/// \brief ���������� �����ִ°�?
	bool IsOpen() { return m_File == INVALID_HANDLE_VALUE; }
	
	
public:
	/// \brief �α׸� �ۼ��Ѵ�.
	void Write(LPCTSTR log);

	/// \brief ���� ������ �α׸� �ۼ��Ѵ�.
	template <typename... Arguments>
	void WriteFormatted(LPCTSTR fmt, const Arguments&... arg)
	{
		CFixedString<2048> fixedString;
		Write(fixedString.FormatSafe(fmt, arg...));
	}

	/// \brief ������ ���⸦ ����
	void Close();
};