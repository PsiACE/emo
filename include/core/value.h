#ifndef emo_core_value_h
#define emo_core_value_h

#include "core/common.h"

// I don't want a `nil` value.
typedef enum {
	VAL_BOOL,	// default `false`
	VAL_NUMBER, // default `0`
} ValueType;

typedef struct {
	ValueType type;
	union {
		bool boolean;
		double number;
	} as;
} Value;

#define IS_BOOL(value) ((value).type == VAL_BOOL)
#define IS_NUMBER(value) ((value).type == VAL_NUMBER)

#define AS_BOOL(value) ((value).as.boolean)
#define AS_NUMBER(value) ((value).as.number)

#define BOOL_VAL(value) ((Value){VAL_BOOL, {.boolean = value}})
#define NUMBER_VAL(value) ((Value){VAL_NUMBER, {.number = value}})

typedef struct {
	int count;
	int capacity;
	Value *values;
} ValueArray;

bool values_equal(Value a, Value b);
void init_value_array(ValueArray *array);
void write_value_array(ValueArray *array, Value value);
void free_value_array(ValueArray *array);

void print_value(Value value);

#endif
