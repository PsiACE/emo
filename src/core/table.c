#include <stdlib.h>
#include <string.h>

#include "core/memory.h"
#include "core/object.h"
#include "core/table.h"
#include "core/value.h"

#define TABLE_MAX_LOAD 0.75

void init_table(Table *table)
{
	table->count = 0;
	table->capacity = -1;
	table->entries = NULL;
}

void free_table(Table *table)
{
	FREE_ARRAY(Entry, table->entries, table->capacity + 1);
	init_table(table);
}

static Entry *find_entry(Entry *entries, int capacity, Value key)
{
	uint32_t index = hash_value(key) & capacity;
	Entry *tombstone = NULL;

	for (;;) {
		Entry *entry = &entries[index];

		if (IS_META(entry->key)) {
			if (IS_META(entry->value)) {
				// Empty entry.
				return tombstone != NULL ? tombstone : entry;
			} else {
				// We found a tombstone.
				if (tombstone == NULL)
					tombstone = entry;
			}
		} else if (values_equal(key, entry->key)) {
			// We found the key.
			return entry;
		}

		index = (index + 1) & capacity;
	}
}

bool table_get(Table *table, Value key, Value *value)
{
	if (table->count == 0)
		return false;

	Entry *entry = find_entry(table->entries, table->capacity, key);
	if (IS_META(entry->key))
		return false;

	*value = entry->value;
	return true;
}

static void adjust_capacity(Table *table, int capacity)
{
	Entry *entries = ALLOCATE(Entry, capacity + 1);
	for (int i = 0; i <= capacity; i++) {
		entries[i].key = META_VAL;
		entries[i].value = META_VAL;
	}

	table->count = 0;
	for (int i = 0; i <= table->capacity; i++) {
		Entry *entry = &table->entries[i];
		if (IS_META(entry->key))
			continue;

		Entry *dest = find_entry(entries, capacity, entry->key);
		dest->key = entry->key;
		dest->value = entry->value;
		table->count++;
	}

	FREE_ARRAY(Entry, table->entries, table->capacity + 1);
	table->entries = entries;
	table->capacity = capacity;
}

bool table_set(Table *table, Value key, Value value)
{
	if (table->count + 1 > (table->capacity + 1) * TABLE_MAX_LOAD) {
		int capacity = GROW_CAPACITY(table->capacity + 1) - 1;
		adjust_capacity(table, capacity);
	}

	Entry *entry = find_entry(table->entries, table->capacity, key);

	bool isNewKey = IS_META(entry->key);
	if (isNewKey && IS_META(entry->value))
		table->count++;

	entry->key = key;
	entry->value = value;
	return isNewKey;
}

bool table_delete(Table *table, Value key)
{
	if (table->count == 0)
		return false;

	// Find the entry.
	Entry *entry = find_entry(table->entries, table->capacity, key);
	if (IS_META(entry->key))
		return false;

	// Place a tombstone in the entry.
	entry->key = META_VAL;
	entry->value = BOOL_VAL(true);

	return true;
}

void table_add_all(Table *from, Table *to)
{
	for (int i = 0; i <= from->capacity; i++) {
		Entry *entry = &from->entries[i];
		if (!IS_META(entry->key)) {
			table_set(to, entry->key, entry->value);
		}
	}
}

ObjString *table_find_string(Table *table, const char *chars, int length, uint32_t hash)
{
	if (table->count == 0)
		return NULL;

	uint32_t index = hash & table->capacity;

	for (;;) {
		Entry *entry = &table->entries[index];

		if (IS_META(entry->key))
			return NULL;

		ObjString *string = AS_STRING(entry->key);
		if (string->length == length && memcmp(string->chars, chars, length) == 0) {
			// We found it.
			return string;
		}

		index = (index + 1) & table->capacity;
	}
}
