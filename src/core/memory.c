#include <stdlib.h>

#include "core/common.h"
#include "core/memory.h"

void *reallocate(void *previous, size_t oldSize, size_t newSize)
{
	if (newSize == 0) {
		free(previous);
		return NULL;
	}

	return realloc(previous, newSize);
}
