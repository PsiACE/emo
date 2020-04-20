#include <stdio.h>
#include <string.h>

#include "core/memory.h"
#include "core/object.h"
#include "core/value.h"
#include "core/vm.h"

#define ALLOCATE_OBJ(type, objectType) (type *)allocate_object(sizeof(type), objectType)

static Obj *allocate_object(size_t size, ObjType type)
{
	Obj *object = (Obj *)reallocate(NULL, 0, size);
	object->type = type;
	object->isMarked = false;
	object->next = vm.objects;
	vm.objects = object;

#ifdef DEBUG_LOG_GC
	printf("%p allocate %zu for %d\n", (void *)object, size, type);
#endif

	return object;
}

ObjClosure *new_closure(ObjFunction *function)
{
	ObjUpvalue **upvalues = ALLOCATE(ObjUpvalue *, function->upvalueCount);
	for (int i = 0; i < function->upvalueCount; ++i) {
		upvalues[i] = NULL;
	}

	ObjClosure *closure = ALLOCATE_OBJ(ObjClosure, OBJ_CLOSURE);
	closure->function = function;
	closure->upvalues = upvalues;
	closure->upvalueCount = function->upvalueCount;
	return closure;
}

ObjNative *new_native(NativeFn function)
{
	ObjNative *native = ALLOCATE_OBJ(ObjNative, OBJ_NATIVE);
	native->function = function;
	return native;
}

ObjFunction *new_function()
{
	ObjFunction *function = ALLOCATE_OBJ(ObjFunction, OBJ_FUNCTION);

	function->arity = 0;
	function->upvalueCount = 0;
	function->name = NULL;
	init_chunk(&function->chunk);
	return function;
}

// TODO: Maybe we can try `halfsiphash-1-3` or `ahash`
// Now, we use the FNV 1a hash algorithm.
static uint32_t hash_chars(const char *key, int length)
{
	register uint32_t hash = 2166136261u;

	for (int i = length; i != 0; i--) {
		hash ^= key[i];
		hash *= 16777619;
	}

	hash ^= key[0];
	hash *= 16777619;

	return hash;
}

ObjString *make_string(int length)
{
	ObjString *string = (ObjString *)allocate_object(sizeof(ObjString) + length + 1, OBJ_STRING);
	string->length = length;
	return string;
}

ObjString *copy_string(const char *chars, int length)
{
	ObjString *string = make_string(length);

	memcpy(string->chars, chars, length);
	string->chars[length] = '\0';

	ObjString *hashString = hash_string(string);

	return hashString;
}

ObjString *hash_string(ObjString *string)
{
	uint32_t hash = hash_chars(string->chars, string->length);
	ObjString *interned = table_find_string(&vm.strings, string->chars, string->length, hash);

	if (interned != NULL)
		return interned;

	string->hash = hash;

	push(OBJ_VAL(string));
	table_set(&vm.strings, OBJ_VAL(string), META_VAL);
	pop();

	return string;
}

ObjUpvalue *new_upvalue(Value *slot)
{
	ObjUpvalue *upvalue = ALLOCATE_OBJ(ObjUpvalue, OBJ_UPVALUE);
	upvalue->closed = META_VAL;
	upvalue->location = slot;
	upvalue->next = NULL;
	return upvalue;
}

static void print_function(ObjFunction *function)
{
	if (function->name == NULL) {
		printf("<script>");
		return;
	}
	printf("<fn %s>", function->name->chars);
}

void print_object(Value value)
{
	switch (OBJ_TYPE(value)) {
	case OBJ_CLOSURE:
		print_function(AS_CLOSURE(value)->function);
		break;
	case OBJ_FUNCTION:
		print_function(AS_FUNCTION(value));
		break;
	case OBJ_NATIVE:
		printf("<native fn>");
		break;
	case OBJ_STRING:
		printf("%s", AS_CSTRING(value));
		break;
	case OBJ_UPVALUE:
		printf("<upvalue>");
		break;
	}
}
