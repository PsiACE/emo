#ifndef emo_core_math_h
#define emo_core_math_h

#include <stdint.h>

static union {
	char c[4];
	unsigned long mylong;
} endian_test = {{'l', '?', '?', 'b'}};
#define _LE_ (*(char *)&endian_test.mylong == l)

#define M_E 2.7182818284590452354
#define M_LOG2E 1.4426950408889634074
#define M_LOG10E 0.43429448190325182765
#define M_LN2 0.69314718055994530942
#define M_LN10 2.30258509299404568402
#define M_PI 3.14159265358979323846
#define M_PI_2 1.57079632679489661923
#define M_PI_4 0.78539816339744830962
#define M_1_PI 0.31830988618379067154
#define M_2_PI 0.63661977236758134308
#define M_2_SQRTPI 1.12837916709551257390
#define M_SQRT2 1.41421356237309504880
#define M_SQRT1_2 0.70710678118654752440

#if __LE__

typedef union {
	uint32_t bin;

	struct {
		uint32_t sign : 1;
		uint32_t exp : 8;
		uint32_t fraction : 23;
	} parts __attribute__((packed));
} float32;

typedef union {
	uint64_t bin;

	struct {
		uint64_t sign : 1;
		uint64_t exp : 11;
		uint64_t fraction : 52;
	} parts __attribute__((packed));
} float64;

#else

typedef union {
	uint32_t bin;

	struct {
		uint32_t fraction : 23;
		uint32_t exp : 8;
		uint32_t sign : 1;
	} parts __attribute__((packed));
} float32;

typedef union {
	uint64_t bin;

	struct {
		uint64_t fraction : 52;
		uint64_t exp : 11;
		uint64_t sign : 1;
	} parts __attribute__((packed));
} float64;

#endif

#if __SIZEOF_DOUBLE__ == 4

#define float_t double
#define FLOAT_FRACTION_SIZE 23
#define FLOAT_BIAS 0x7f
#define TRUNC_MASK UINT32_C(0x007fffff)
#define TAYLOR_DEGREE_EXP 13
#define TAYLOR_DEGREE_LOG 31

typedef union {
	float_t val;
	float32 data;
} float_u;

#elif __SIZEOF_DOUBLE__ == 8

#define float_t double
#define FLOAT_FRACTION_SIZE 52
#define FLOAT_BIAS 0x3ff
#define TRUNC_MASK UINT64_C(0x000fffffffffffff)
#define TAYLOR_DEGREE_EXP 21
#define TAYLOR_DEGREE_LOG 63

typedef union {
	float_t val;
	float64 data;
} float_u;

#endif

float_t trunc(float_t val);
float_t mod(float_t dividend, float_t divisor);
float_t exp(float_t index);
float_t log(float_t arg);
float_t pow(float_t base, float_t exponent);

#endif
