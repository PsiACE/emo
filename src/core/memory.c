#include <stdlib.h>

#include "core/common.h"
#include "core/compiler.h"
#include "core/memory.h"
#include "core/vm.h"

#ifdef DEBUG_LOG_GC
#include "core/debug.h"
#include <stdio.h>
#endif

#define GC_HEAP_GROW_FACTOR 2

void *reallocate(void *previous, size_t oldSize, size_t newSize)
{
	vm.bytesAllocated += newSize - oldSize;

	if (newSize > oldSize) {
#ifdef DEBUG_STRESS_GC
		collect_garbage();
#endif
		if (vm.bytesAllocated > vm.nextGC) {
			collect_garbage();
		}
	}

	if (newSize == 0) {
		free(previous);
		return NULL;
	}

	return realloc(previous, newSize);
}

void mark_object(Obj *object)
{
	if (object == NULL)
		return;
	if (object->isMarked)
		return;

#ifdef DEBUG_LOG_GC
	printf("%p mark ", (void *)object);
	print_value(OBJ_VAL(object));
	printf("\n");
#endif

	object->isMarked = true;

	if (vm.grayCapacity < vm.grayCount + 1) {
		vm.grayCapacity = GROW_CAPACITY(vm.grayCapacity);
		vm.grayStack = realloc(vm.grayStack, sizeof(Obj *) * vm.grayCapacity);
	}

	vm.grayStack[vm.grayCount++] = object;
}

void mark_value(Value value)
{
	if (!IS_OBJ(value))
		return;
	mark_object(AS_OBJ(value));
}

static void mark_array(ValueArray *array)
{
	for (int i = 0; i < array->count; ++i) {
		mark_value(array->values[i]);
	}
}

static void free_object(Obj *object)
{
#ifdef DEBUG_LOG_GC
	printf("%p free type %d\n", (void *)object, object->type);
#endif

	switch (object->type) {
	case OBJ_CLOSURE: {
		ObjClosure *closure = (ObjClosure *)object;
		FREE_ARRAY(ObjUpvalue *, closure->upvalues, closure->upvalueCount);
		FREE(ObjClosure, object);
		break;
	}
	case OBJ_FUNCTION: {
		ObjFunction *function = (ObjFunction *)object;
		free_chunk(&function->chunk);
		FREE(ObjFunction, object);
		break;
	}
	case OBJ_NATIVE:
		FREE(ObjNative, object);
		break;
	case OBJ_STRING: {
		ObjString *string = (ObjString *)object;
		reallocate(object, sizeof(ObjString) + string->length + 1, 0);
		break;
	}
	case OBJ_UPVALUE:
		FREE(ObjUpvalue, object);
		break;
	}
}

static void blacken_object(Obj *object)
{
#ifdef DEBUG_LOG_GC
	printf("%p blacken ", (void *)object);
	print_value(OBJ_VAL(object));
	printf("\n");
#endif

	switch (object->type) {
	case OBJ_CLOSURE: {
		ObjClosure *closure = (ObjClosure *)object;
		mark_object((Obj *)closure->function);
		for (int i = 0; i < closure->upvalueCount; ++i) {
			mark_object((Obj *)closure->upvalues[i]);
		}
		break;
	}
	case OBJ_FUNCTION: {
		ObjFunction *function = (ObjFunction *)object;
		mark_object((Obj *)function->name);
		mark_array(&function->chunk.constants);
		break;
	}
	case OBJ_UPVALUE:
		mark_value(((ObjUpvalue *)object)->closed);
		break;
	case OBJ_NATIVE:
	case OBJ_STRING:
		break;
	}
}

static void mark_roots()
{
	for (Value *slot = vm.stack; slot < vm.stackTop; ++slot) {
		mark_value(*slot);
	}

	for (int i = 0; i < vm.frameCount; ++i) {
		mark_object((Obj *)vm.frames[i].closure);
	}

	for (ObjUpvalue *upvalue = vm.openUpvalues; upvalue != NULL; upvalue = upvalue->next) {
		mark_object((Obj *)upvalue);
	}

	mark_table(&vm.globals);
	mark_compiler_roots();
}

static void trace_references()
{
	while (vm.grayCount > 0) {
		Obj *object = vm.grayStack[--vm.grayCount];
		blacken_object(object);
	}
}

static void sweep()
{
	Obj *previous = NULL;
	Obj *object = vm.objects;
	while (object != NULL) {
		if (object->isMarked) {
			object->isMarked = false;
			previous = object;
			object = object->next;
		} else {
			Obj *unreached = object;

			object = object->next;
			if (previous != NULL) {
				previous->next = object;
			} else {
				vm.objects = object;
			}

			free_object(unreached);
		}
	}
}

void collect_garbage()
{
#ifdef DEBUG_LOG_GC
	printf("-- gc begin\n");
	size_t before = vm.bytesAllocated;
#endif

	mark_roots();
	trace_references();
	table_remove_white(&vm.strings);
	sweep();

	vm.nextGC = vm.bytesAllocated * GC_HEAP_GROW_FACTOR;

#ifdef DEBUG_LOG_GC
	printf("-- gc end\n");
	printf("   collected %zu bytes (from %zu to %zu) next at %zu\n", before - vm.bytesAllocated, before,
		   vm.bytesAllocated, vm.nextGC);
#endif
}

void free_objects()
{
	Obj *object = vm.objects;
	while (object != NULL) {
		Obj *next = object->next;
		free_object(object);
		object = next;
	}

	free(vm.grayStack);
}
