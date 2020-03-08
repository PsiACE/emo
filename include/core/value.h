#ifndef emo_value_h
#define emo_value_h

#include "core/common.h"

typedef double Value;

typedef struct {
	int count;
	int capacity;
	Value *values;
} ValueArray;

void init_value_array(ValueArray *array);
void write_value_array(ValueArray *array, Value value);
void free_value_array(ValueArray *array);

void print_value(Value value);

#endif
