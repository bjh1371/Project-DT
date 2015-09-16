////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file Math.h
/// \author bjh1371
/// \date 2015/07/09
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

namespace MathUtil
{
	/// \brief �־��� ������ ���ų� ū 2�� �¼� �߿� ���� ���� ���� ��ȯ�Ѵ�.
	inline unsigned int GetNearestPowerOfTwo(unsigned int value)
	{
		const unsigned MANTISSA_BIT_DEPTH = 23;
		const unsigned MANTISSA_MASK = (1 << MANTISSA_BIT_DEPTH) - 1;

		(*(float*)&value) = (float)value;

		value += MANTISSA_MASK;
		value >>= 23;
		value -= 127;

		return 1 << value;
	}
}
