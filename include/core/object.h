#ifndef emo_core_object_h
#define emo_core_object_h

#include "core/common.h"
#include "core/value.h"

#define OBJ_TYPE(value) (AS_OBJ(value)->type)
#define IS_STRING(value) is_obj_type(value, OBJ_STRING)
#define AS_STRING(value) ((ObjString *)AS_OBJ(value))
#define AS_CSTRING(value) (((ObjString *)AS_OBJ(value))->chars)

typedef enum {
	OBJ_STRING,
} ObjType;

struct sObj {
	ObjType type;
	struct sObj *next;
};

struct sObjString {
	Obj obj;
	bool ownsChars; // <--
	int length;
	uint32_t hash;
	// char *chars;
	char chars[];
};

// ObjString *take_string(char *chars, int length);
ObjString *make_string(int length);
ObjString *copy_string(const char *chars, int length);
ObjString *take_string(ObjString *string);
ObjString *hash_string(ObjString *string);

void print_object(Value value);

static inline bool is_obj_type(Value value, ObjType type)
{
	return IS_OBJ(value) && AS_OBJ(value)->type == type;
}

#endif
