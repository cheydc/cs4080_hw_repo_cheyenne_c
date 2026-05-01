#ifndef clox_compiler_h
#define clox_compiler_h

#include "object.h"
#include "vm.h"

typedef enum {
  TYPE_FUNCTION,
  TYPE_SCRIPT,
} FunctionType;

ObjFunction* compile(const char* source);

#endif