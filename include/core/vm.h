#ifndef emo_core_vm_h
#define emo_core_vm_h

#include <stdint.h>

#include "core/chunk.h"
#include "core/object.h"
#include "core/table.h"
#include "core/value.h"

#define FRAMES_MAX 64
#define STACK_MAX (FRAMES_MAX * UINT8_COUNT)

typedef struct {
	ObjClosure *closure;
	uint8_t *ip;
	Value *slots;
} CallFrame;

typedef struct {
	CallFrame frames[FRAMES_MAX];
	int frameCount;
	Chunk *chunk;
	uint8_t *ip;
	Value *stack;
	Value *stackTop;
	int stackCapacity;
	Table globals;
	Table strings;
	ObjUpvalue *openUpvalues;
	Obj *objects;
	size_t bytesAllocated;
	size_t nextGC;
	int grayCount;
	int grayCapacity;
	Obj **grayStack;
} VM;

extern VM vm;

typedef enum { INTERPRET_OK, INTERPRET_COMPILE_ERROR, INTERPRET_RUNTIME_ERROR } InterpretResult;

void init_vm();
void free_vm();

InterpretResult interpret(const char *source);
void push(Value value);
Value pop();

#endif
