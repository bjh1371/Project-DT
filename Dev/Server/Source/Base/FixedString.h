////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file FixedString.h
/// \author bjh1371
/// \date 2015/07/06
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "FixedToken.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \class CFixedString
/// \brief 스택에 사이즈를 정해놓은 스트링
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template <size_t MAX_CAPACITY>
class CFixedString
{
private:
	TCHAR m_Buffer[MAX_CAPACITY + 1]; ///< 버퍼


public:
	/// \brief 생성자
	CFixedString() { m_Buffer[0] = 0; }

	/// \brief 생성자
	template <typename T>
	CFixedString(const T& value)
	{
		Set(value);
	}
	
	/// \brief 소멸자
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
	/// \brief 문자열을 생성해서 반환한다.
	template <typename... Arguments>
	inline LPCTSTR FormatSafe(LPCTSTR fmt, const Arguments&... arg)
	{
		std::initializer_list<CFixedToken> argList = { arg... };
		return CFixedToken::BuildSafe(m_Buffer, ARRAYSIZE(m_Buffer), fmt, argList);
	}

	/// \brief 버퍼를 반환
	inline LPTSTR Get()
	{
		return m_Buffer;
	}

	/// \brief 형변환 연산자
	inline operator LPCTSTR()
	{
		return m_Buffer;
	}


public:
	/// \brief 날짜를 반환
	inline LPCTSTR MakeDate(TCHAR dateSep = '-');

	/// \brief 시간을 반환
	inline LPCTSTR MakeTime(TCHAR timeSep = ':');

	/// \brief 날짜 시간을 반환
	inline LPCTSTR MakeDateTime(TCHAR dateSep = '-', TCHAR dateTimeSep = ' ', TCHAR timeSep = ':');
};

#include "FixedString.inl"