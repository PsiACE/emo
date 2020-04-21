#include <stdarg.h>
#include <stdio.h>

#include "core/math.h"

static void math_error(const char *format, ...)
{
	va_list args;
	va_start(args, format);
	vfprintf(stderr, format, args);
	va_end(args);
	fputs("\n", stderr);
}

#ifdef float32_t

float32_t trunc(float32_t val)
{
	float32_u v;
	int32_t exp;

	v.val = val;
	exp = v.data.parts.exp - FLOAT32_BIAS;

	if (exp < 0) {
		v.data.parts.exp = 0;
		v.data.parts.fraction = 0;
	} else if (exp >= FLOAT32_FRACTION_SIZE) {
		if (exp == 1024) {
			math_error("val %g is +inf, -inf or NaN.", val);
		}
	} else {
		v.data.parts.fraction &= ~(UINT32_C(0x007fffff) >> exp);
	}

	return v.val;
}

float32_t fmod(float32_t dividend, float32_t divisor)
{
	float32_t quotient = trunc(dividend / divisor);

	return (dividend - quotient * divisor);
}

#else

float64_t fmod(float64_t dividend, float64_t divisor)
{
	float64_t quotient = trunc(dividend / divisor);

	return (dividend - quotient * divisor);
}

float64_t trunc(float64_t val)
{
	float64_u v;
	int32_t exp;

	v.val = val;
	exp = v.data.parts.exp - FLOAT64_BIAS;

	if (exp < 0) {
		v.data.parts.exp = 0;
		v.data.parts.fraction = 0;
	} else if (exp >= FLOAT64_FRACTION_SIZE) {
		if (exp == 1024) {
			math_error("val %g is +inf, -inf or NaN.", val);
		}
	} else {
		v.data.parts.fraction &= ~(UINT64_C(0x000fffffffffffff) >> exp);
	}

	return v.val;
}

#endif
