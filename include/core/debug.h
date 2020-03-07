#ifndef emo_debug_h
#define emo_debug_h

#include "core/chunk.h"

void disassemble_chunk(Chunk *chunk, const char *name);
int disassemble_instruction(Chunk *chunk, int offset);

#endif
