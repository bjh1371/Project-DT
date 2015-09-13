////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file FixedString.h
/// \author bjh1371
/// \date 2015/07/06
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "FixedToken.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \class CFixedString
/// \brief ���ÿ� ����� ���س��� ��Ʈ��
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template <size_t MAX_CAPACITY>
class CFixedString
{
private:
	TCHAR m_Buffer[MAX_CAPACITY + 1]; ///< ����


public:
	/// \brief ������
	CFixedString() { m_Buffer[0] = 0; }

	/// \brief ������
	template <typename T>
	CFixedString(const T& value)
	{
		Set(value);
	}
	
	/// \brief �Ҹ���
	~CFixedString() {}

	
public:
	inline LPCTSTR Set(char value) {               _stprintf_s(m_Buffer, ARRAYSIZE(m_Buffer),_T("%c"), value);  return m_Buffer;  }
	inline LPCTSTR Set(unsigned char value) {      _stprintf_s(m_Buffer, ARRAYSIZE(m_Buffer),_T("%u"), value);  return m_Buffer;  }
	inline LPCTSTR Set(short value) {              _stprintf_s(m_Buffer, ARRAYSIZE(m_Buffer),_T("%d"), value);  return m_Buffer;  }
	inline LPCTSTR Set(unsigned short value) {     _stprintf_s(m_Buffer, ARRAYSIZE(m_Buffer),_T("%u"), value);  return m_Buffer;  }
	inline LPCTSTR Set(int value) {                _stprintf_s(m_Buffer, ARRAYSIZE(m_Buffer),_T("%d"), value);  return m_Buffer;  }
	inline LPCTSTR Set(unsigned int value) {       _stprintf_s(m_Buffer, ARRAYSIZE(m_Buffer),_T("%u"), value);  return m_Buffer;  }
	inline LPCTSTR Set(float value) {              _stprintf_s(m_Buffer, ARRAYSIZE(m_Buffer),_T("%f"), value);  return m_Buffer;  }
	inline LPCTSTR Set(double value) {             _stprintf_s(m_Buffer, ARRAYSIZE(m_Buffer),_T("%f"), value);  return m_Buffer;  }
	inline LPCTSTR Set(long value) {               _stprintf_s(m_Buffer, ARRAYSIZE(m_Buffer),_T("%ld"), value); return m_Buffer;  }
	inline LPCTSTR Set(unsigned long value) {      _stprintf_s(m_Buffer, ARRAYSIZE(m_Buffer),_T("%lu"), value); return m_Buffer;  }
	inline LPCTSTR Set(long long value) {          _stprintf_s(m_Buffer, ARRAYSIZE(m_Buffer),_T("%lld"), value); return m_Buffer; }
	inline LPCTSTR Set(unsigned long long value) { _stprintf_s(m_Buffer, ARRAYSIZE(m_Buffer),_T("%llu"), value); return m_Buffer; }
	inline LPCTSTR Set(LPCTSTR value) {            _tcsncpy_s(m_Buffer, ARRAYSIZE(m_Buffer), value ? value : L(""), _TRUNCATE); return m_Buffer;   }


public:
	/// \brief ���ڿ��� �����ؼ� ��ȯ�Ѵ�.
	template <typename... Arguments>
	inline LPCTSTR FormatSafe(LPCTSTR fmt, const Arguments&... arg)
	{
		std::initializer_list<CFixedToken> argList = { arg... };
		return CFixedToken::BuildSafe(m_Buffer, ARRAYSIZE(m_Buffer), fmt, argList);
	}

	/// \brief ���۸� ��ȯ
	inline LPTSTR Get()
	{
		return m_Buffer;
	}

	/// \brief ����ȯ ������
	inline operator LPCTSTR()
	{
		return m_Buffer;
	}


public:
	/// \brief ��¥�� ��ȯ
	inline LPCTSTR MakeDate(TCHAR dateSep = '-');

	/// \brief �ð��� ��ȯ
	inline LPCTSTR MakeTime(TCHAR timeSep = ':');

	/// \brief ��¥ �ð��� ��ȯ
	inline LPCTSTR MakeDateTime(TCHAR dateSep = '-', TCHAR dateTimeSep = ' ', TCHAR timeSep = ':');
};

#include "FixedString.inl"