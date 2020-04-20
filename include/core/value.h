#ifndef emo_core_value_h
#define emo_core_value_h

#include "core/common.h"

typedef struct sObj Obj;
typedef struct sObjString ObjString;

// TODO: I don't want a `nil` value. But may need a type for nothing.
// So, internal use only a meta type.
typedef enum {
	VAL_BOOL,	// default `false`
	VAL_NUMBER, // default `0`
	VAL_META,	// always `()`.
	VAL_OBJ,
} ValueType;

typedef struct {
	ValueType type;
	union {
		bool boolean;
		double number;
		char *meta;
		Obj *obj;
	} as;
} Value;

#define IS_BOOL(value) ((value).type == VAL_BOOL)
#define IS_NUMBER(value) ((value).type == VAL_NUMBER)
#define IS_META(value) ((value).type == VAL_META)
#define IS_OBJ(value) ((value).type == VAL_OBJ)

#define AS_BOOL(value) ((value).as.boolean)
#define AS_NUMBER(value) ((value).as.number)
#define AS_OBJ(value) ((value).as.obj)

#define BOOL_VAL(value) ((Value){VAL_BOOL, {.boolean = value}})
#define NUMBER_VAL(value) ((Value){VAL_NUMBER, {.number = value}})
#define META_VAL ((Value){VAL_META, {.meta = "()"}})
#define OBJ_VAL(object) ((Value){VAL_OBJ, {.obj = (Obj *)object}})

typedef struct {
	int count;
	int capacity;
	Value *values;
} ValueArray;

uint32_t hash_value(Value value);

bool values_equal(Value a, Value b);
void init_value_array(ValueArray *array);
void write_value_array(ValueArray *array, Value value);
void free_value_array(ValueArray *array);

void print_value(Value value);

#endif
