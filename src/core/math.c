#include <stdarg.h>
#include <stdio.h>

#include "core/math.h"

static float_t factorials[21] = {1,
								 2,
								 6,
								 24,
								 120,
								 720,
								 5040,
								 40320,
								 362880,
								 3628800,
								 39916800,
								 479001600,
								 6227020800.0L,
								 87178291200.0L,
								 1307674368000.0L,
								 20922789888000.0L,
								 355687428096000.0L,
								 6402373705728000.0L,
								 121645100408832000.0L,
								 2432902008176640000.0L,
								 51090942171709440000.0L};

static void math_error(const char *format, ...)
{
	va_list args;
	va_start(args, format);
	vfprintf(stderr, format, args);
	va_end(args);
	fputs("\n", stderr);
}

float_t trunc(float_t val)
{
	float_u v;
	int32_t exp;

	v.val = val;
	exp = v.data.parts.exp - FLOAT_BIAS;

	if (exp < 0) {
		v.data.parts.exp = 0;
		v.data.parts.fraction = 0;
	} else if (exp >= FLOAT_FRACTION_SIZE) {
		if (exp == 1024) {
			math_error("val %g is +inf, -inf or NaN.", val);
		}
	} else {
		v.data.parts.fraction &= ~(TRUNC_MASK >> exp);
	}

	return v.val;
}

float_t mod(float_t dividend, float_t divisor)
{
	float_t quotient = trunc(dividend / divisor);

	return (dividend - quotient * divisor);
}

static inline float_t taylor_exp(float_t index)
{
	float_t ret = 1;
	float_t nom = 1;

	for (unsigned int i = 0; i < TAYLOR_DEGREE_EXP; i++) {
		nom *= index;
		ret += nom / factorials[i];
	}

	return ret;
}

float_t exp(float_t index)
{
	float_t f;
	float_t i;
	float_u r;

	i = trunc(index * M_LOG2E);
	f = index * M_LOG2E - i;

	r.val = taylor_exp(M_LN2 * f);
	r.data.parts.exp += i;
	return r.val;
}

static float_t taylor_log(float_t arg)
{
	float_t ret = 0;
	float_t num = 1;

	for (unsigned int i = 1; i <= TAYLOR_DEGREE_LOG; i++) {
		num *= arg;

		if ((i % 2) == 0)
			ret += num / i;
		else
			ret -= num / i;
	}

	return ret;
}

float_t log(float_t arg)
{
	float_u m;
	int e;

	m.val = arg;
	e = m.data.parts.exp - (FLOAT_BIAS - 1);
	m.data.parts.exp = FLOAT_BIAS - 1;

	return -taylor_log(m.val - 1.0) + e / M_LOG2E;
}

float_t pow(float_t base, float_t exponent)
{
	return exp(log(base) * exponent);
}
