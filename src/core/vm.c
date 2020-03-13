#include <stdio.h>

#include "core/common.h"
#include "core/compiler.h"
#include "core/debug.h"
#include "core/memory.h"
#include "core/vm.h"

VM vm;

static void reset_stack()
{
	vm.stackCount = 0;
}

void init_vm()
{
	vm.stack = NULL;
	vm.stackCapacity = 0;
	reset_stack();
}

void free_vm() {}

void push(Value value)
{
	if (vm.stackCapacity < vm.stackCount + 1) {
		int oldCapacity = vm.stackCapacity;
		vm.stackCapacity = GROW_CAPACITY(oldCapacity);
		vm.stack = GROW_ARRAY(vm.stack, Value, oldCapacity, vm.stackCapacity);
	}

	vm.stack[vm.stackCount] = value;
	vm.stackCount++;
}

Value pop()
{
	vm.stackCount--;
	return vm.stack[vm.stackCount];
}

static InterpretResult run()
{
#define READ_BYTE() (*vm.ip++)
#define READ_CONSTANT() (vm.chunk->constants.values[READ_BYTE()])

#define BINARY_OP(op)                                                                                                  \
	do {                                                                                                               \
		double b = pop();                                                                                              \
		double a = pop();                                                                                              \
		push(a op b);                                                                                                  \
	} while (false)

	for (;;) {
#ifdef DEBUG_TRACE_EXECUTION
		printf("          ");
		for (Value *slot = vm.stack; slot < vm.stackTop; slot++) {
			printf("[ ");
			print_value(*slot);
			printf(" ]");
		}
		printf("\n");
#endif
		uint8_t instruction;
		switch (instruction = READ_BYTE()) {
		case OP_CONSTANT: {
			Value constant = READ_CONSTANT();
			push(constant);
			break;
		}
		case OP_ADD:
			BINARY_OP(+);
			break;
		case OP_SUBTRACT:
			BINARY_OP(-);
			break;
		case OP_MULTIPLY:
			BINARY_OP(*);
			break;
		case OP_DIVIDE:
			BINARY_OP(/);
			break;
		case OP_NEGATE:
			push(-pop());
			break;
		case OP_CONSTANT_LONG: {
			uint32_t index = READ_BYTE();
			index |= READ_BYTE() << 8;
			index |= READ_BYTE() << 16;
			push(vm.chunk->constants.values[index]);
			break;
		}
		case OP_RETURN: {
			print_value(pop());
			printf("\n");
			return INTERPRET_OK;
		}
		}
	}

#undef READ_BYTE
#undef READ_CONSTANT
#undef BINARY_OP
}

InterpretResult interpret(const char *source)
{
	compile(source);
	return INTERPRET_OK;
}
