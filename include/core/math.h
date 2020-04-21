#ifndef emo_core_math_h
#define emo_core_math_h

#include <stdint.h>

static union {
	char c[4];
	unsigned long mylong;
} endian_test = {{'l', '?', '?', 'b'}};
#define _LE_ (*(char *)&endian_test.mylong == l)

#define FLOAT32_FRACTION_SIZE 23
#define FLOAT64_FRACTION_SIZE 52

#define FLOAT32_BIAS 0x7f
#define FLOAT64_BIAS 0x3ff

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
#define float32_t double
#elif __SIZEOF_DOUBLE__ == 8
#define float64_t double
#endif

#ifdef float32_t

typedef union {
	float32_t val;
	float32 data;
} float32_u;

float32_t trunc(float32_t val);
float32_t fmod(float32_t dividend, float32_t divisor);

#endif

#ifdef float64_t

typedef union {
	float64_t val;
	float64 data;
} float64_u;

float64_t trunc(float64_t val);
float64_t fmod(float64_t dividend, float64_t divisor);

#endif
#endif
