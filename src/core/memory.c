#include <stdlib.h>

#include "core/common.h"
#include "core/memory.h"
#include "core/vm.h"

void *reallocate(void *previous, size_t oldSize, size_t newSize)
{
	if (newSize == 0) {
		free(previous);
		return NULL;
	}

	return realloc(previous, newSize);
}

static void free_object(Obj *object)
{
	switch (object->type) {
	case OBJ_FUNCTION: {
		ObjFunction *function = (ObjFunction *)object;
		free_chunk(&function->chunk);
		FREE(ObjFunction, object);
		break;
	}
	case OBJ_STRING: {
		ObjString *string = (ObjString *)object;
		// FREE_ARRAY(char, string->chars, string->length + 1);
		// FREE(ObjString, object);
		reallocate(object, sizeof(ObjString) + string->length + 1, 0);
		break;
	}
	}
}

void free_objects()
{
	Obj *object = vm.objects;
	while (object != NULL) {
		Obj *next = object->next;
		free_object(object);
		object = next;
	}
}
