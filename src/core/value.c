#include <stdio.h>
#include <string.h>

#include "core/memory.h"
#include "core/object.h"
#include "core/value.h"

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
	case VAL_OBJ: {
		ObjString *aString = AS_STRING(a);
		ObjString *bString = AS_STRING(b);
		return aString->length == bString->length && memcmp(aString->chars, bString->chars, aString->length) == 0;
	}
	}

	return false;
}
