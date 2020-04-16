#ifndef emo_core_object_h
#define emo_core_object_h

#include "core/chunk.h"
#include "core/common.h"
#include "core/value.h"

#define OBJ_TYPE(value) (AS_OBJ(value)->type)
#define IS_CLOSURE(value) is_obj_type(value, OBJ_CLOSURE)
#define IS_FUNCTION(value) is_obj_type(value, OBJ_FUNCTION)
#define IS_NATIVE(value) is_obj_type(value, OBJ_NATIVE)
#define IS_STRING(value) is_obj_type(value, OBJ_STRING)

#define AS_CLOSURE(value) ((ObjClosure *)AS_OBJ(value))
#define AS_FUNCTION(value) ((ObjFunction *)AS_OBJ(value))
#define AS_NATIVE(value) (((ObjNative *)AS_OBJ(value))->function)
#define AS_STRING(value) ((ObjString *)AS_OBJ(value))
#define AS_CSTRING(value) (((ObjString *)AS_OBJ(value))->chars)

typedef enum {
	OBJ_CLOSURE,
	OBJ_FUNCTION,
	OBJ_NATIVE,
	OBJ_STRING,
	OBJ_UPVALUE,
} ObjType;

struct sObj {
	ObjType type;
	struct sObj *next;
};

typedef struct {
	Obj obj;
	int arity;
	int upvalueCount;
	Chunk chunk;
	ObjString *name;
} ObjFunction;

typedef Value (*NativeFn)(int argCount, Value *args);

typedef struct {
	Obj obj;
	NativeFn function;
} ObjNative;

struct sObjString {
	Obj obj;
	bool ownsChars; // <--
	int length;
	uint32_t hash;
	// char *chars;
	char chars[];
};

typedef struct sUpvalue {
	Obj obj;
	Value *location;
	Value closed;
	struct sUpvalue *next;
} ObjUpvalue;

typedef struct {
	Obj obj;
	ObjFunction *function;
	ObjUpvalue **upvalues;
	int upvalueCount;
} ObjClosure;

ObjFunction *new_function();
ObjNative *new_native(NativeFn function);
ObjClosure *new_closure(ObjFunction *function);
// ObjString *take_string(char *chars, int length);
ObjString *make_string(int length);
ObjString *copy_string(const char *chars, int length);
ObjString *take_string(ObjString *string);
ObjString *hash_string(ObjString *string);
ObjUpvalue *new_upvalue(Value *slot);

void print_object(Value value);

static inline bool is_obj_type(Value value, ObjType type)
{
	return IS_OBJ(value) && AS_OBJ(value)->type == type;
}

#endif
