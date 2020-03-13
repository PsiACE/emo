#ifndef emo_vm_h
#define emo_vm_h

#include <stdint.h>

#include "core/chunk.h"
#include "core/value.h"

typedef struct {
	Chunk *chunk;
	uint8_t *ip;
	Value *stack;
	int stackCount;
	int stackCapacity;
} VM;

typedef enum { INTERPRET_OK, INTERPRET_COMPILE_ERROR, INTERPRET_RUNTIME_ERROR } InterpretResult;

void init_vm();
void free_vm();

InterpretResult interpret(const char *source);
void push(Value value);
Value pop();

#endif
