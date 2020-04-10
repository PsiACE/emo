#ifndef emo_core_table_h
#define emo_core_table_h

#include "core/common.h"
#include "core/value.h"

typedef struct {
	ObjString *key;
	Value value;
} Entry;

typedef struct {
	int count;
	int capacity;
	Entry *entries;
} Table;

void init_table(Table *table);
void free_table(Table *table);
bool table_get(Table *table, ObjString *key, Value *value);
bool table_set(Table *table, ObjString *key, Value value);
bool table_delete(Table *table, ObjString *key);
void table_add_all(Table *from, Table *to);
ObjString *table_find_string(Table *table, const char *chars, int length, uint32_t hash);

#endif
