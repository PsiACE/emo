#ifndef emo_core_table_h
#define emo_core_table_h

#include "core/common.h"
#include "core/value.h"

typedef struct {
	Value key;
	Value value;
} Entry;

typedef struct {
	int count;
	int capacity;
	Entry *entries;
} Table;

void init_table(Table *table);
void free_table(Table *table);
bool table_get(Table *table, Value key, Value *value);
bool table_set(Table *table, Value key, Value value);
bool table_delete(Table *table, Value key);
void table_add_all(Table *from, Table *to);
ObjString *table_find_string(Table *table, const char *chars, int length, uint32_t hash);

#endif
