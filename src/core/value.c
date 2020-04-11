#include <stdio.h>
#include <string.h>

#include "core/memory.h"
#include "core/object.h"
#include "core/value.h"

static uint32_t hash_double(double value)
{
	union BitCast {
		double value;
		uint32_t ints[2];
	};

	union BitCast cast;
	cast.value = (value) + 1.0;
	return cast.ints[0] + cast.ints[1];
}

uint32_t hash_value(Value value)
{
	switch (value.type) {
	case VAL_BOOL:
		return AS_BOOL(value) ? 3 : 5;
	case VAL_NUMBER:
		return hash_double(AS_NUMBER(value));
	case VAL_OBJ:
		return AS_STRING(value)->hash;
	case VAL_META:
		return 0;
	}
	return 0;
}

void init_value_array(ValueArray *array)
{
	array->values = NULL;
	array->capacity = 0;
	array->count = 0;
}

void write_value_array(ValueArray *array, Value value)
{
	if (array->capacity < array->count + 1) {
		int oldCapacity = array->capacity;
		array->capacity = GROW_CAPACITY(oldCapacity);
		array->values = GROW_ARRAY(array->values, Value, oldCapacity, array->capacity);
	}

	array->values[array->count] = value;
	array->count++;
}

void free_value_array(ValueArray *array)
{
	FREE_ARRAY(Value, array->values, array->capacity);
	init_value_array(array);
}

void print_value(Value value)
{
	switch (value.type) {
	case VAL_BOOL:
		printf(AS_BOOL(value) ? "true" : "false");
		break;
	case VAL_NUMBER:
		printf("%g", AS_NUMBER(value));
		break;
	case VAL_META:
		printf("<meta>");
		break;
	case VAL_OBJ:
		print_object(value);
		break;
	}
}

bool values_equal(Value a, Value b)
{
	if (a.type != b.type)
		return false;

	switch (a.type) {
	case VAL_BOOL:
		return AS_BOOL(a) == AS_BOOL(b);
	case VAL_NUMBER:
		return AS_NUMBER(a) == AS_NUMBER(b);
	case VAL_META:
		return true; // `meta` is a default meta.
	case VAL_OBJ:
		return AS_OBJ(a) == AS_OBJ(b);
	}

	return false;
}
