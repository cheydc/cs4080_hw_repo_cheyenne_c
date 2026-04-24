#include <stdio.h>
#include <string.h>

#include "memory.h"
#include "object.h"
#include "table.h"
#include "value.h"
#include "vm.h"

static uint32_t hashString(const char* key, int length) {
  uint32_t hash = 2166136261u;
  for (int i = 0; i < length; i++) {
    hash ^= (uint8_t)key[i];
    hash *= 16777619;
  }
  return hash;
}

static Obj* allocateObject(size_t size, ObjType type) {
  Obj* object = (Obj*)reallocate(NULL, 0, size);
  object->type = type;
  object->next = vm.objects;
  vm.objects   = object;
  return object;
}

#define ALLOCATE_OBJ(type, objectType) \
  (type*)allocateObject(sizeof(type), objectType)

static ObjString* allocateString(char* chars, int length,
                                 uint32_t hash, bool ownsChars) {
  ObjString* string  = ALLOCATE_OBJ(ObjString, OBJ_STRING);
  string->length     = length;
  string->chars      = chars;
  string->hash       = hash;
  string->ownsChars  = ownsChars;
  tableSet(&vm.strings, OBJ_VAL(string), NIL_VAL);
  return string;
}

ObjString* takeString(char* chars, int length) {
  uint32_t hash = hashString(chars, length);
  return allocateString(chars, length, hash, true);
}

ObjString* copyString(const char* chars, int length) {
  uint32_t hash = hashString(chars, length);
  for (int i = 0; i < vm.strings.capacity; i++) {
    Entry* entry = &vm.strings.entries[i];
    if (IS_NIL(entry->key)) continue;
    ObjString* s = AS_STRING(entry->key);
    if (s->length == length && s->hash == hash &&
        memcmp(s->chars, chars, length) == 0) {
      return s;
    }
  }
  return allocateString((char*)chars, length, hash, false);
}

void printObject(Value value) {
  switch (OBJ_TYPE(value)) {
    case OBJ_STRING:
      printf("%.*s", AS_STRING(value)->length, AS_STRING(value)->chars);
      break;
  }
}