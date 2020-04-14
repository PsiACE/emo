#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "core/common.h"
#include "core/compiler.h"
#include "core/debug.h"
#include "core/memory.h"
#include "core/object.h"
#include "core/value.h"
#include "core/vm.h"

VM vm;

static void reset_stack()
{
	vm.stackCount = 0;
	vm.frameCount = 0;
}

static void runtime_error(const char *format, ...)
{
	va_list args;
	va_start(args, format);
	vfprintf(stderr, format, args);
	va_end(args);
	fputs("\n", stderr);

	CallFrame *frame = &vm.frames[vm.frameCount - 1];
	size_t instructionOffset = frame->ip - frame->function->chunk.code - 1;
	//   int line = frame->function->chunk.lines[instruction];
	int line = get_line(&frame->function->chunk.lines, instructionOffset);
	fprintf(stderr, "[line %d] in script\n", line);

	reset_stack();
}

void init_vm()
{
	vm.stack = NULL;
	vm.stackCapacity = 0;
	reset_stack();
	vm.objects = NULL;
	init_table(&vm.globalNames);
	init_table(&vm.strings);
}

void free_vm()
{
	free_table(&vm.globalNames);
	free_table(&vm.strings);
	free_objects();
}

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

// First, we check to see if the value on top of the stack is a number.
static Value peek(int distance)
{
	int look = vm.stackCount;
	return vm.stack[look - 1 - distance];
}

static bool is_falsey(Value value)
{
	return (IS_BOOL(value) && !AS_BOOL(value));
}

static void concatenate()
{
	ObjString *b = AS_STRING(pop());
	ObjString *a = AS_STRING(pop());

	int length = a->length + b->length;
	ObjString *string = make_string(length);
	memcpy(string->chars, a->chars, a->length);
	memcpy(string->chars + a->length, b->chars, b->length);
	string->chars[length] = '\0';
	ObjString *result = take_string(string);
	// char *chars = ALLOCATE(char, length + 1);
	// memcpy(chars, a->chars, a->length);
	// memcpy(chars + a->length, b->chars, b->length);
	// chars[length] = '\0';

	// ObjString *result = take_string(chars, length);
	push(OBJ_VAL(result));
}

static InterpretResult run()
{
	CallFrame *frame = &vm.frames[vm.frameCount - 1];

#define READ_BYTE() (*frame->ip++)
#define READ_SHORT() (frame->ip += 2, (uint16_t)((frame->ip[-2] << 8) | frame->ip[-1]))
#define READ_CONSTANT() (frame->function->chunk.constants.values[READ_BYTE()])
#define READ_STRING() AS_STRING(READ_CONSTANT())

#define BINARY_OP(valueType, op)                                                                                       \
	do {                                                                                                               \
		if (!IS_NUMBER(peek(0)) || !IS_NUMBER(peek(1))) {                                                              \
			runtime_error("Operands must be numbers.");                                                                \
			return INTERPRET_RUNTIME_ERROR;                                                                            \
		}                                                                                                              \
                                                                                                                       \
		double b = AS_NUMBER(pop());                                                                                   \
		double a = AS_NUMBER(pop());                                                                                   \
		push(valueType(a op b));                                                                                       \
	} while (false)

	for (;;) {
#ifdef DEBUG_TRACE_EXECUTION
		printf("          ");
		for (Value *slot = vm.stack; slot < &vm.stack[vm.stackCount]; slot++) {
			printf("[ ");
			print_value(*slot);
			printf(" ]");
		}
		printf("\n");
		disassemble_instruction(&frame->function->chunk, (int)(frame->ip - frame->function->chunk.code));
#endif
		uint8_t instruction;
		switch (instruction = READ_BYTE()) {
		case OP_CONSTANT: {
			Value constant = READ_CONSTANT();
			push(constant);
			break;
		}
		case OP_TRUE:
			push(BOOL_VAL(true));
			break;
		case OP_FALSE:
			push(BOOL_VAL(false));
			break;
		case OP_POP:
			pop();
			break;
		case OP_META:
			push(META_VAL);
			break;
		case OP_GET_LOCAL: {
			uint8_t slot = READ_BYTE();
			push(frame->slots[slot]);
			break;
		}
		case OP_SET_LOCAL: {
			uint8_t slot = READ_BYTE();
			frame->slots[slot] = peek(0);
			break;
		}
		case OP_GET_GLOBAL: {
			Value value = vm.globalValues.values[READ_BYTE()];
			if (IS_META(value)) {
				runtime_error("Undefined variable.");
				return INTERPRET_RUNTIME_ERROR;
			}
			push(value);
			break;
		}
		case OP_DEFINE_GLOBAL: {
			vm.globalValues.values[READ_BYTE()] = pop();
			break;
		}
		case OP_SET_GLOBAL: {
			uint8_t index = READ_BYTE();
			if (IS_META(vm.globalValues.values[index])) {
				runtime_error("Undefined variable.");
				return INTERPRET_RUNTIME_ERROR;
			}
			vm.globalValues.values[index] = peek(0);
			break;
		}
		case OP_EQUAL: {
			Value b = pop();
			Value a = pop();
			push(BOOL_VAL(values_equal(a, b)));
			break;
		}
		case OP_GREATER:
			BINARY_OP(BOOL_VAL, >);
			break;
		case OP_LESS:
			BINARY_OP(BOOL_VAL, <);
			break;
		case OP_ADD: {
			if (IS_STRING(peek(0)) && IS_STRING(peek(1))) {
				concatenate();
			} else if (IS_NUMBER(peek(0)) && IS_NUMBER(peek(1))) {
				double b = AS_NUMBER(pop());
				double a = AS_NUMBER(pop());
				push(NUMBER_VAL(a + b));
			} else {
				runtime_error("Operands must be two numbers or two strings.");
				return INTERPRET_RUNTIME_ERROR;
			}
			break;
		}
		case OP_MULTIPLY:
			BINARY_OP(NUMBER_VAL, *);
			break;
		case OP_DIVIDE:
			BINARY_OP(NUMBER_VAL, /);
			break;
		case OP_NOT:
			push(BOOL_VAL(is_falsey(pop())));
			break;
		case OP_NEGATE:
			if (!IS_NUMBER(peek(0))) {
				runtime_error("Operand must be a number.");
				return INTERPRET_RUNTIME_ERROR;
			}

			push(NUMBER_VAL(-AS_NUMBER(pop())));
			break;
		case OP_PRINT: {
			print_value(pop());
			printf("\n");
			break;
		}
		case OP_JUMP: {
			uint16_t offset = READ_SHORT();
			frame->ip += offset;
			break;
		}
		case OP_JUMP_IF_FALSE: {
			uint16_t offset = READ_SHORT();
			if (is_falsey(peek(0)))
				frame->ip += offset;

			break;
		}
		case OP_LOOP: {
			uint16_t offset = READ_SHORT();
			frame->ip -= offset;
			break;
		}
		case OP_CONSTANT_LONG: {
			uint32_t index = READ_BYTE();
			index |= READ_BYTE() << 8;
			index |= READ_BYTE() << 16;
			push(vm.chunk->constants.values[index]);
			break;
		}
		case OP_RETURN: {
			Value arbitraryValue = pop();
			print_value(arbitraryValue);
			printf("\n");
			push(arbitraryValue);
			return INTERPRET_OK;
		}
		}
	}

#undef READ_BYTE
#undef READ_SHORT
#undef READ_CONSTANT
#undef READ_STRING
#undef BINARY_OP
}

InterpretResult interpret(const char *source)
{
	ObjFunction *function = compile(source);
	if (function == NULL)
		return INTERPRET_COMPILE_ERROR;

	push(OBJ_VAL(function));
	CallFrame *frame = &vm.frames[vm.frameCount++];
	frame->function = function;
	frame->ip = function->chunk.code;
	frame->slots = vm.stack;

	return run();
}
