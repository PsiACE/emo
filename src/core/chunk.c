#include <stdio.h>
#include <stdlib.h>

#include "core/chunk.h"
#include "core/memory.h"

void init_line_record_array(LineRecordArray *array)
{
	array->linemarks = NULL;
	array->capacity = 0;
	array->count = 0;
}

void write_line_record_array(LineRecordArray *array, int line)
{
	if (array->capacity < array->count + 1) {
		int oldCapacity = array->capacity;
		array->capacity = GROW_CAPACITY(oldCapacity);
		array->linemarks = GROW_ARRAY(array->linemarks, LineRecord, oldCapacity, array->capacity);
	}

	encode_line_record_array(array, line);
}

void encode_line_record_array(LineRecordArray *array, int line)
{
	if (array->count == 0) {
		array->linemarks[0].linemark = line;
		array->linemarks[0].offset = 0;
		array->count++;
	}

	if (line == array->linemarks[array->count - 1].linemark) {
		array->linemarks[array->count - 1].offset++;
	} else {
		int index = array->count;
		array->linemarks[index].linemark = line;
		array->linemarks[index].offset = 1;
		array->count++;
	}
}

void free_line_record_array(LineRecordArray *array)
{
	FREE_ARRAY(LineRecord, array->linemarks, array->capacity);
	init_line_record_array(array);
}

void init_chunk(Chunk *chunk)
{
	chunk->count = 0;
	chunk->capacity = 0;
	chunk->code = NULL;
	// chunk->lines = NULL;
	init_line_record_array(&chunk->lines);
	init_value_array(&chunk->constants);
}

void free_chunk(Chunk *chunk)
{
	FREE_ARRAY(uint8_t, chunk->code, chunk->capacity);
	// FREE_ARRAY(int, chunk->lines, chunk->capacity);
	free_line_record_array(&chunk->lines);
	free_value_array(&chunk->constants);
	init_chunk(chunk);
}

void write_chunk(Chunk *chunk, uint8_t byte, int line)
{
	if (chunk->capacity < chunk->count + 1) {
		int oldCapacity = chunk->capacity;
		chunk->capacity = GROW_CAPACITY(oldCapacity);
		chunk->code = GROW_ARRAY(chunk->code, uint8_t, oldCapacity, chunk->capacity);
		// chunk->lines = GROW_ARRAY(chunk->lines, int, oldCapacity, chunk->capacity);
	}

	write_line_record_array(&chunk->lines, line);

	chunk->code[chunk->count] = byte;
	// chunk->lines[chunk->count] = line;
	chunk->count++;
}

int add_constant(Chunk *chunk, Value value)
{
	write_value_array(&chunk->constants, value);
	return chunk->constants.count - 1;
}

void write_constant(Chunk *chunk, Value value, int line)
{
	int index = add_constant(chunk, value);
	if (index < 256) {
		write_chunk(chunk, OP_CONSTANT, line);
		write_chunk(chunk, (uint8_t)index, line);
	} else {
		write_chunk(chunk, OP_CONSTANT_LONG, line);
		write_chunk(chunk, (uint8_t)(index & 0xff), line);
		write_chunk(chunk, (uint8_t)((index >> 8) & 0xff), line);
		write_chunk(chunk, (uint8_t)((index >> 16) & 0xff), line);
	}
}

// int get_line(LineRecordArray *array, int offset)
// {
// 	int start = 0;
// 	int end = array->count - 1;

// 	for (;;) {
// 		int mid = (start + end) / 2;
// 		if (offset > array->linemarks[mid].offset) {
// 			end = mid - 1;
// 		} else if (mid == array->count - 1 || offset > array->linemarks[mid + 1].offset) {
// 			return array->linemarks[mid].linemark;
// 		} else {
// 			start = mid + 1;
// 		}
// 	}
// }

int get_line(LineRecordArray *array, int offset)
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
