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
	object->next = vm.objects;
	vm.objects = object;
	return object;
}

static uint32_t hash_chars(const char *key, int length)
{
	uint32_t hash = 2166136261u;

	for (int i = 0; i < length; i++) {
		hash ^= key[i];
		hash *= 16777619;
	}

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

ObjString *take_string(ObjString *string)
{
	uint32_t hash = hash_chars(string->chars, string->length);
	ObjString *interned = table_find_string(&vm.strings, string->chars, string->length, hash);

	if (interned != NULL) {
		reallocate(string, sizeof(ObjString) + string->length + 1, 0);
		return interned;
	}

	string->hash = hash;

	table_set(&vm.strings, OBJ_VAL(string), META_VAL);

	return string;
}

ObjString *hash_string(ObjString *string)
{
	uint32_t hash = hash_chars(string->chars, string->length);
	ObjString *interned = table_find_string(&vm.strings, string->chars, string->length, hash);

	if (interned != NULL)
		return interned;

	string->hash = hash;

	table_set(&vm.strings, OBJ_VAL(string), META_VAL);

	return string;
}

void print_object(Value value)
{
	switch (OBJ_TYPE(value)) {
	case OBJ_STRING:
		printf("%s", AS_CSTRING(value));
		break;
	}
}

// static ObjString *allocate_string(char *chars, int length)
// {
// 	ObjString *string = ALLOCATE_OBJ(ObjString, OBJ_STRING);
// 	string->length = length;
// 	string->chars = chars;

// 	return string;
// }

// ObjString *take_string(char *chars, int length)
// {
// 	return allocate_string(chars, length);
// }

// ObjString *copy_string(const char *chars, int length)
// {
// 	char *heapChars = ALLOCATE(char, length + 1);
// 	memcpy(heapChars, chars, length);
// 	heapChars[length] = '\0';

// 	return allocate_string(heapChars, length);
// }
