#ifndef emo_core_chunk_h
#define emo_core_chunk_h

#include "core/common.h"
#include "core/value.h"

typedef enum {
	OP_CONSTANT,
	OP_TRUE,
	OP_FALSE,
	OP_EQUAL,
	OP_GREATER,
	OP_LESS,
	OP_ADD,
	OP_SUBTRACT,
	OP_MULTIPLY,
	OP_DIVIDE,
	OP_NOT,
	OP_NEGATE,
	OP_CONSTANT_LONG,
	OP_RETURN,
} OpCode;

typedef struct {
	int linemark;
	int offset;
} LineRecord;

typedef struct {
	int count;
	int capacity;
	LineRecord *linemarks;
} LineRecordArray;

typedef struct {
	int count;
	int capacity;
	uint8_t *code;
	// int *lines;
	LineRecordArray lines;
	ValueArray constants;
} Chunk;

void init_line_record_array(LineRecordArray *array);
void write_line_record_array(LineRecordArray *array, int line);
void encode_line_record_array(LineRecordArray *array, int line);
void free_line_record_array(LineRecordArray *array);

void init_chunk(Chunk *chunk);
void write_chunk(Chunk *chunk, uint8_t byte, int line);
void free_chunk(Chunk *chunk);

int add_constant(Chunk *chunk, Value value);
void write_constant(Chunk *chunk, Value value, int line);

#endif
