#ifndef emo_core_vm_h
#define emo_core_vm_h

#include <stdint.h>

#include "core/chunk.h"
#include "core/table.h"
#include "core/value.h"

typedef struct {
	Chunk *chunk;
	uint8_t *ip;
	Value *stack;
	int stackCount;
	int stackCapacity;
	// Table globals;
	Table globalNames;
	ValueArray globalValues;
	Table strings;
	Obj *objects;
} VM;

extern VM vm;

typedef enum { INTERPRET_OK, INTERPRET_COMPILE_ERROR, INTERPRET_RUNTIME_ERROR } InterpretResult;

void init_vm();
void free_vm();

InterpretResult interpret(const char *source);
void push(Value value);
Value pop();

#endif
