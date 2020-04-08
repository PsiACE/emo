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
