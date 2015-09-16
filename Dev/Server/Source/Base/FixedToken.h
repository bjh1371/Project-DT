////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file FixedToken.h
/// \author bjh1371
/// \date 2015/07/06
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "FixedString.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \class CFixedToken
/// \brief 고정길이 토큰
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CFixedToken
{
private:
	TCHAR m_Buffer[256]; ///< 버퍼


public:
	/// \brief 생성자
	template <typename T>
	CFixedToken(const T& value) { Set(value); }
	
	/// \brief 소멸자
	~CFixedToken() {}
	
	
public:
	inline LPCTSTR Set(char value)               { _stprintf_s(m_Buffer, ARRAYSIZE(m_Buffer), _T("%c"), value);  return m_Buffer; }
	inline LPCTSTR Set(unsigned char value)      { _stprintf_s(m_Buffer, ARRAYSIZE(m_Buffer), _T("%u"), value);  return m_Buffer; }
	inline LPCTSTR Set(short value)              { _stprintf_s(m_Buffer, ARRAYSIZE(m_Buffer), _T("%d"), value);  return m_Buffer; }
	inline LPCTSTR Set(unsigned short value)     { _stprintf_s(m_Buffer, ARRAYSIZE(m_Buffer), _T("%u"), value);  return m_Buffer; }
	inline LPCTSTR Set(int value)                { _stprintf_s(m_Buffer, ARRAYSIZE(m_Buffer), _T("%d"), value);  return m_Buffer; }
	inline LPCTSTR Set(unsigned int value)       { _stprintf_s(m_Buffer, ARRAYSIZE(m_Buffer), _T("%u"), value);  return m_Buffer; }
	inline LPCTSTR Set(float value)              { _stprintf_s(m_Buffer, ARRAYSIZE(m_Buffer), _T("%f"), value);  return m_Buffer; }
	inline LPCTSTR Set(double value)             { _stprintf_s(m_Buffer, ARRAYSIZE(m_Buffer), _T("%f"), value);  return m_Buffer; }
	inline LPCTSTR Set(long value)               { _stprintf_s(m_Buffer, ARRAYSIZE(m_Buffer), _T("%ld"), value); return m_Buffer; }
	inline LPCTSTR Set(unsigned long value)      { _stprintf_s(m_Buffer, ARRAYSIZE(m_Buffer), _T("%lu"), value); return m_Buffer; }
	inline LPCTSTR Set(long long value)          { _stprintf_s(m_Buffer, ARRAYSIZE(m_Buffer), _T("%lld"), value); return m_Buffer; }
	inline LPCTSTR Set(unsigned long long value) { _stprintf_s(m_Buffer, ARRAYSIZE(m_Buffer), _T("%llu"), value); return m_Buffer; }
	inline LPCTSTR Set(LPCTSTR value)            { _tcsncpy_s(m_Buffer, ARRAYSIZE(m_Buffer), value ? value : _T(""), _TRUNCATE); return m_Buffer; }


public:
	/// \brief 버퍼를 얻는다.
	inline LPCTSTR Get() const { return m_Buffer; }


public:
	/// \brief 문자열을 파싱해서 만든다.
	static LPCTSTR BuildSafe(LPTSTR buf, size_t size, LPCTSTR fmt, const std::initializer_list<CFixedToken>& args);
};


