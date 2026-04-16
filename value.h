#ifndef clox_value_h
#define clox_value_h

#include "common.h"

/* Value type used by the table (typically number or object pointer) */
typedef struct Obj Obj;

typedef struct {
  double number;
} Value;

/* Macros used in your code */
#define NUMBER_VAL(x) ((Value){ .number = (double)(x) })
#define AS_NUMBER(value) ((value).number)

#endif