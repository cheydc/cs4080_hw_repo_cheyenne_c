#include <stdio.h>
#include <string.h>

#include "memory.h"
#include "object.h"
#include "value.h"
#include "vm.h"

#define ALLOCATE_OBJ_FLEX(type, objectType, extraBytes) \
  (type*)allocateObject(sizeof(type) + (extraBytes), objectType)

static Obj* allocateObject(size_t size, ObjType type) {
  Obj* object = (Obj*)reallocate(NULL, 0, size);
  object->type = type;
  object->next = vm.objects;
  vm.objects = object;
  return object;
}

static ObjString* allocateString(const char* chars, int length) {
  ObjString* string =
      ALLOCATE_OBJ_FLEX(ObjString, OBJ_STRING, sizeof(char) * (length + 1));
  string->length = length;
  memcpy(string->chars, chars, length);
  string->chars[length] = '\0';
  return string;
}

ObjString* takeString(char* chars, int length) {
  ObjString* string = allocateString(chars, length);
  FREE_ARRAY(char, chars, length + 1);
  return string;
}

ObjString* copyString(const char* chars, int length) {
  return allocateString(chars, length);
}

void printObject(Value value) {
  switch (OBJ_TYPE(value)) {
    case OBJ_STRING:
      printf("%s", AS_CSTRING(value));
      break;
  }
}