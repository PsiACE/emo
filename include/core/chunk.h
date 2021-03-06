#ifndef emo_core_chunk_h
#define emo_core_chunk_h

#include "core/common.h"
#include "core/value.h"

typedef enum {
	OP_CONSTANT,
	OP_TRUE,
	OP_FALSE,
	OP_POP,
	OP_META, // `meta` means noting
	OP_GET_LOCAL,
	OP_SET_LOCAL,
	OP_GET_GLOBAL,
	OP_DEFINE_GLOBAL,
	OP_SET_GLOBAL,
	OP_GET_UPVALUE,
	OP_SET_UPVALUE,
	OP_EQUAL,
	OP_GREATER,
	OP_LESS,
	OP_ADD,
	OP_MULTIPLY,
	OP_DIVIDE,
	OP_MODULO,
	OP_POW,
	OP_NOT,
	OP_NEGATE,
	OP_PRINT,
	OP_JUMP,
	OP_JUMP_IF_FALSE,
	OP_LOOP,
	OP_CALL,
	OP_CLOSURE,
	OP_CLOSE_UPVALUE,
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

int get_line(LineRecordArray *array, int offset);

#endif
