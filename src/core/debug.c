#include <stdio.h>

#include "core/chunk.h"
#include "core/debug.h"
#include "core/object.h"
#include "core/value.h"

void disassemble_chunk(Chunk *chunk, const char *name)
{
	printf("== %s ==\n", name);

	for (int offset = 0; offset < chunk->count;) {
		offset = disassemble_instruction(chunk, offset);
	}
}

static int constant_instruction(const char *name, Chunk *chunk, int offset)
{
	uint8_t constant = chunk->code[offset + 1];
	printf("%-16s %4d '", name, constant);
	print_value(chunk->constants.values[constant]);
	printf("'\n");
	return offset + 2;
}

static int long_constant_instruction(const char *name, Chunk *chunk, int offset)
{
	uint32_t constant = chunk->code[offset + 1] | (chunk->code[offset + 2] << 8) | (chunk->code[offset + 3] << 16);
	printf("%-16s %4u '", name, constant);
	print_value(chunk->constants.values[constant]);
	printf("'\n");
	return offset + 4;
}

static int simple_instruction(const char *name, int offset)
{
	printf("%s\n", name);
	return offset + 1;
}

static int byte_instruction(const char *name, Chunk *chunk, int offset)
{
	uint8_t slot = chunk->code[offset + 1];
	printf("%-16s %4d\n", name, slot);
	return offset + 2;
}

static int jump_instruction(const char *name, int sign, Chunk *chunk, int offset)
{
	uint16_t jump = (uint16_t)(chunk->code[offset + 1] << 8);
	jump |= chunk->code[offset + 2];
	printf("%-16s %4d -> %d\n", name, offset, offset + 3 + sign * jump);
	return offset + 3;
}

int disassemble_instruction(Chunk *chunk, int offset)
{
	printf("%04d ", offset);

	int line = get_line(&chunk->lines, offset);

	if (offset > 0 && line == get_line(&chunk->lines, offset - 1)) {
		printf("   | ");
	} else {
		printf("%4d ", line);
	}

	uint8_t instruction = chunk->code[offset];
	switch (instruction) {
	case OP_CONSTANT:
		return constant_instruction("OP_CONSTANT", chunk, offset);
	case OP_TRUE:
		return simple_instruction("OP_TRUE", offset);
	case OP_FALSE:
		return simple_instruction("OP_FALSE", offset);
	case OP_POP:
		return simple_instruction("OP_POP", offset);
	case OP_META:
		return simple_instruction("OP_META", offset);
	case OP_GET_LOCAL:
		return byte_instruction("OP_GET_LOCAL", chunk, offset);
	case OP_SET_LOCAL:
		return byte_instruction("OP_SET_LOCAL", chunk, offset);
	case OP_GET_GLOBAL:
		return constant_instruction("OP_GET_GLOBAL", chunk, offset);
	case OP_GET_UPVALUE:
		return byte_instruction("OP_GET_UPVALUE", chunk, offset);
	case OP_SET_UPVALUE:
		return byte_instruction("OP_SET_UPVALUE", chunk, offset);
	case OP_DEFINE_GLOBAL:
		return constant_instruction("OP_DEFINE_GLOBAL", chunk, offset);
	case OP_SET_GLOBAL:
		return constant_instruction("OP_SET_GLOBAL", chunk, offset);
	case OP_EQUAL:
		return simple_instruction("OP_EQUAL", offset);
	case OP_GREATER:
		return simple_instruction("OP_GREATER", offset);
	case OP_LESS:
		return simple_instruction("OP_LESS", offset);
	case OP_ADD:
		return simple_instruction("OP_ADD", offset);
	case OP_MULTIPLY:
		return simple_instruction("OP_MULTIPLY", offset);
	case OP_DIVIDE:
		return simple_instruction("OP_DIVIDE", offset);
	case OP_NOT:
		return simple_instruction("OP_NOT", offset);
	case OP_NEGATE:
		return simple_instruction("OP_NEGATE", offset);
	case OP_PRINT:
		return simple_instruction("OP_PRINT", offset);
	case OP_JUMP:
		return jump_instruction("OP_JUMP", 1, chunk, offset);
	case OP_JUMP_IF_FALSE:
		return jump_instruction("OP_JUMP_IF_FALSE", 1, chunk, offset);
	case OP_LOOP:
		return jump_instruction("OP_LOOP", -1, chunk, offset);
	case OP_CALL:
		return byte_instruction("OP_CALL", chunk, offset);
	case OP_CLOSURE: {
		offset++;
		uint8_t constant = chunk->code[offset++];
		printf("%-16s %4d ", "OP_CLOSURE", constant);
		print_value(chunk->constants.values[constant]);
		printf("\n");

		ObjFunction *function = AS_FUNCTION(chunk->constants.values[constant]);
		for (int j = 0; j < function->upvalueCount; j++) {
			int isLocal = chunk->code[offset++];
			int index = chunk->code[offset++];
			printf("%04d      |                     %s %d\n", offset - 2, isLocal ? "local" : "upvalue", index);
		}
		return offset;
	}
	case OP_CLOSE_UPVALUE:
		return simple_instruction("OP_CLOSE_UPVALUE", offset);
	case OP_CONSTANT_LONG:
		return long_constant_instruction("OP_CONSTANT_LONG", chunk, offset);
	case OP_RETURN:
		return simple_instruction("OP_RETURN", offset);
	default:
		printf("Unknown opcode %d\n", instruction);
		return offset + 1;
	}
}
