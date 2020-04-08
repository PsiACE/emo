#ifndef emo_core_compiler_h
#define emo_core_compiler_h

#include "core/object.h"
#include "core/vm.h"

bool compile(const char *source, Chunk *chunk);

#endif
