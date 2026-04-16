#include <string.h>

#include "memory.h"
#include "table.h"
#include "value.h"

#define TABLE_MAX_LOAD 0.75

void initTable(Table* table) {
  table->count = 0;
  table->capacity = 0;
  table->entries = NULL;
}

void freeTable(Table* table) {
  FREE_ARRAY(Entry, table->entries, table->capacity);
  initTable(table);
}

static uint32_t hashValue(Value value) {
  switch (value.type) {
    case VAL_BOOL:
      return AS_BOOL(value) ? 1 : 2;

    case VAL_NIL:
      return 3;

    case VAL_NUMBER: {
      union {
        double num;
        uint64_t bits;
      } data;
      data.num = AS_NUMBER(value);
      return (uint32_t)(data.bits ^ (data.bits >> 32));
    }

    case VAL_OBJ:
      if (IS_STRING(value)) {
        return AS_STRING(value)->hash;
      }
      return (uint32_t)(uintptr_t)AS_OBJ(value);
  }
  return 0;
}

static Entry* findEntry(Entry* entries, int capacity, Value key) {
  uint32_t index = hashValue(key) % capacity;
  Entry* tombstone = NULL;

  for (;;) {
    Entry* entry = &entries[index];

    if (IS_NIL(entry->key)) {
      if (IS_NIL(entry->value)) {
        return tombstone != NULL ? tombstone : entry;
      } else {
        if (tombstone == NULL) tombstone = entry;
      }
    } else if (valuesEqual(entry->key, key)) {
      return entry;
    }

    index = (index + 1) % capacity;
  }
}

static void adjustCapacity(Table* table, int capacity) {
  Entry* entries = ALLOCATE(Entry, capacity);

  for (int i = 0; i < capacity; i++) {
    entries[i].key = NIL_VAL;
    entries[i].value = NIL_VAL;
  }

  table->count = 0;

  for (int i = 0; i < table->capacity; i++) {
    Entry* entry = &table->entries[i];
    if (IS_NIL(entry->key)) continue;

    Entry* dest = findEntry(entries, capacity, entry->key);
    dest->key = entry->key;
    dest->value = entry->value;
    table->count++;
  }

  FREE_ARRAY(Entry, table->entries, table->capacity);
  table->entries = entries;
  table->capacity = capacity;
}

bool tableSet(Table* table, Value key, Value value) {
  if (table->count + 1 > table->capacity * TABLE_MAX_LOAD) {
    int capacity = GROW_CAPACITY(table->capacity);
    adjustCapacity(table, capacity);
  }

  Entry* entry = findEntry(table->entries, table->capacity, key);
  bool isNewKey = IS_NIL(entry->key);

  if (isNewKey && IS_NIL(entry->value)) table->count++;

  entry->key = key;
  entry->value = value;
  return isNewKey;
}

bool tableGet(Table* table, Value key, Value* value) {
  if (table->count == 0) return false;

  Entry* entry = findEntry(table->entries, table->capacity, key);
  if (IS_NIL(entry->key)) return false;

  *value = entry->value;
  return true;
}

bool tableDelete(Table* table, Value key) {
  if (table->count == 0) return false;

  Entry* entry = findEntry(table->entries, table->capacity, key);
  if (IS_NIL(entry->key)) return false;

  entry->key = NIL_VAL;
  entry->value = BOOL_VAL(true);
  return true;
}