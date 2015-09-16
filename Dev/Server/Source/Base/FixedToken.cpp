////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file FixedToken.inl
/// \author bjh1371
/// \date 2015/07/06
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "FixedToken.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief 문자열을 생성한다.
/// \param LPCTSTR buf 생성할 버퍼
/// \param size_t size 버퍼 사이즈
/// \param LPCTSTR fmt fmt 
/// \param const std::initializer_list<CFixedToken> & args  인자
/// \return LPCTSTR 생성한 문자열
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
LPCTSTR CFixedToken::BuildSafe(LPTSTR buf, size_t size, LPCTSTR fmt, const std::initializer_list<CFixedToken>& args)
{
	if (buf != nullptr && size > 0 && fmt != nullptr)
	{
		LPTSTR curBuf = buf;
		LPTSTR endBuf = buf + size - 1;

		LPCTSTR curFmt = fmt;
		LPCTSTR endFmt = fmt + _tcslen(fmt);

		size_t argIndex = 0;

		while (curBuf < endBuf && curFmt < endFmt)
		{
			TCHAR cur = *curFmt;
			if (cur != '%')
			{
				*curBuf = cur;
				curBuf += 1;
				curFmt += 1;
			}
			else
			{
				if (argIndex < args.size() && curFmt + 1 < endFmt)
				{
					LPCTSTR arg = (args.begin() + argIndex)->Get();
					size_t tokenSize = _tcslen(arg);

					_tcsncpy_s(curBuf, endBuf - curBuf + 1, arg, _TRUNCATE);

					curBuf = curBuf + std::min<size_t>(endBuf - curBuf, tokenSize);
					curFmt += 2;
					++argIndex;
				}
				else
				{
					*curBuf = cur;
					curBuf += 1;
					curFmt += 1;
				}
			}
		}

		*curBuf = 0;
	}

	return buf;
}