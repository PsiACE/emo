#include <stdio.h>

#include "core/chunk.h"
#include "core/debug.h"
#include "core/value.h"

void disassemble_chunk(Chunk *chunk, const char *name)
{
	printf("== %s ==\n", name);

	for (int offset = 0; offset < chunk->count;) {
		offset = disassemble_instruction(chunk, offset);
	}
}

static int get_line(LineRecordArray *array, int offset)
{
	int offsetLeft = offset;

	for (int index = 0; index < array->count; index++) {
		if (array->linemarks[index].offset > offsetLeft) {
			return array->linemarks[index].linemark;
		}
		offsetLeft -= array->linemarks[index].offset;
	}

	printf("Error : get_line() returns -1 \n");
	return -1;
}

static int constant_instruction(const char *name, Chunk *chunk, int offset)
{
	uint8_t constant = chunk->code[offset + 1];
	printf("%-16s %4d '", name, constant);
	print_value(chunk->constants.values[constant]);
	printf("'\n");
	return offset + 2;
}

static int simple_instruction(const char *name, int offset)
{
	printf("%s\n", name);
	return offset + 1;
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
	case OP_RETURN:
		return simple_instruction("OP_RETURN", offset);
	default:
		printf("Unknown opcode %d\n", instruction);
		return offset + 1;
	}
}
