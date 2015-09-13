////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file Math.h
/// \author bjh1371
/// \date 2015/07/09
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

namespace MathUtil
{
	/// \brief 주어진 수보다 같거나 큰 2의 승수 중에 가장 작은 값을 반환한다.
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
