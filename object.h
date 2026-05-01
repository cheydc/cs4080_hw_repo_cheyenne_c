#ifndef clox_object_h
#define clox_object_h

#include "common.h"
#include "chunk.h"
#include "value.h"

#define OBJ_TYPE(value)        (AS_OBJ(value)->type)

#define IS_FUNCTION(value)     isObjType(value, OBJ_FUNCTION)
#define IS_NATIVE(value)       isObjType(value, OBJ_NATIVE)
#define IS_STRING(value)       isObjType(value, OBJ_STRING)

#define AS_FUNCTION(value)     ((ObjFunction*)AS_OBJ(value))
#define AS_NATIVE(value)       (((ObjNative*)AS_OBJ(value))->function)
#define AS_NATIVE_OBJ(value)   ((ObjNative*)AS_OBJ(value))
#define AS_STRING(value)       ((ObjString*)AS_OBJ(value))
#define AS_CSTRING(value)      (((ObjString*)AS_OBJ(value))->chars)

typedef enum {
  OBJ_FUNCTION,
  OBJ_NATIVE,
  OBJ_STRING,
} ObjType;

struct Obj {
  ObjType type;
  struct Obj* next;
};

typedef struct {
  Obj obj;
  int arity;
  Chunk chunk;
  ObjString* name;
} ObjFunction;

// *** CHALLENGE 3 ***
// NativeResult lets native functions signal a runtime error instead of
// being forced to always return a Value.
typedef struct {
  bool ok;
  union {
    Value       value;
    const char* error;
  } as;
} NativeResult;

static inline NativeResult nativeOk(Value value) {
  NativeResult result;
  result.ok = true;
  result.as.value = value;
  return result;
}

static inline NativeResult nativeError(const char* message) {
  NativeResult result;
  result.ok = false;
  result.as.error = message;
  return result;
}

// *** CHALLENGE 3 ***
// NativeFn now returns NativeResult instead of Value.
typedef NativeResult (*NativeFn)(int argCount, Value* args);

// Challenge 2: ObjNative stores arity for argument count validation.
typedef struct {
  Obj obj;
  int arity;
  NativeFn function;
} ObjNative;

struct ObjString {
  Obj obj;
  int length;
  char* chars;
  uint32_t hash;
};

ObjFunction* newFunction();
ObjNative*   newNative(NativeFn function, int arity);
ObjString*   takeString(char* chars, int length);
ObjString*   copyString(const char* chars, int length);
void         printObject(Value value);

static inline bool isObjType(Value value, ObjType type) {
  return IS_OBJ(value) && AS_OBJ(value)->type == type;
}

#endif