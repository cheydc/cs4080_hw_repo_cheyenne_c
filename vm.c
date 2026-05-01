#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "common.h"
#include "compiler.h"
#include "debug.h"
#include "object.h"
#include "memory.h"
#include "vm.h"

VM vm;

// Challenge 1: save/load macros for the register ip variable in run().
#define SAVE_IP() (frame->ip = ip)
#define LOAD_IP() (ip = frame->ip)

static void resetStack() {
  vm.stackTop = vm.stack;
  vm.frameCount = 0;
}

static void runtimeError(const char* format, ...) {
  va_list args;
  va_start(args, format);
  vfprintf(stderr, format, args);
  va_end(args);
  fputs("\n", stderr);

  for (int i = vm.frameCount - 1; i >= 0; i--) {
    CallFrame* frame = &vm.frames[i];
    ObjFunction* function = frame->function;
    size_t instruction = frame->ip - function->chunk.code - 1;
    fprintf(stderr, "[line %d] in ", function->chunk.lines[instruction]);
    if (function->name == NULL) {
      fprintf(stderr, "script\n");
    } else {
      fprintf(stderr, "%s()\n", function->name->chars);
    }
  }

  resetStack();
}

// =============================================================================
// *** CHALLENGE 4 *** Native function implementations
// =============================================================================

static NativeResult clockNative(int argCount, Value* args) {
  return nativeOk(NUMBER_VAL((double)clock() / CLOCKS_PER_SEC));
}

// --- sqrt(n) -----------------------------------------------------------------
// Returns the square root of n. Errors on non-numbers and negative inputs,
// demonstrating the Challenge 3 error signaling system.
static NativeResult sqrtNative(int argCount, Value* args) {
  if (!IS_NUMBER(args[0]))
    return nativeError("sqrt() requires a number argument.");
  double n = AS_NUMBER(args[0]);
  if (n < 0)
    return nativeError("sqrt() argument must be non-negative.");
  return nativeOk(NUMBER_VAL(sqrt(n)));
}

// --- abs(n) ------------------------------------------------------------------
static NativeResult absNative(int argCount, Value* args) {
  if (!IS_NUMBER(args[0]))
    return nativeError("abs() requires a number argument.");
  return nativeOk(NUMBER_VAL(fabs(AS_NUMBER(args[0]))));
}

// --- floor(n) ----------------------------------------------------------------
static NativeResult floorNative(int argCount, Value* args) {
  if (!IS_NUMBER(args[0]))
    return nativeError("floor() requires a number argument.");
  return nativeOk(NUMBER_VAL(floor(AS_NUMBER(args[0]))));
}

// --- ceil(n) -----------------------------------------------------------------
static NativeResult ceilNative(int argCount, Value* args) {
  if (!IS_NUMBER(args[0]))
    return nativeError("ceil() requires a number argument.");
  return nativeOk(NUMBER_VAL(ceil(AS_NUMBER(args[0]))));
}

// --- min(a, b) ---------------------------------------------------------------
static NativeResult minNative(int argCount, Value* args) {
  if (!IS_NUMBER(args[0]) || !IS_NUMBER(args[1]))
    return nativeError("min() requires two number arguments.");
  double a = AS_NUMBER(args[0]);
  double b = AS_NUMBER(args[1]);
  return nativeOk(NUMBER_VAL(a < b ? a : b));
}

// --- max(a, b) ---------------------------------------------------------------
static NativeResult maxNative(int argCount, Value* args) {
  if (!IS_NUMBER(args[0]) || !IS_NUMBER(args[1]))
    return nativeError("max() requires two number arguments.");
  double a = AS_NUMBER(args[0]);
  double b = AS_NUMBER(args[1]);
  return nativeOk(NUMBER_VAL(a > b ? a : b));
}

// --- str_len(s) --------------------------------------------------------------
// Returns the character length of a string. This fills a real gap in Lox
// since the language has no other way to inspect a string's length.
static NativeResult strLenNative(int argCount, Value* args) {
  if (!IS_STRING(args[0]))
    return nativeError("str_len() requires a string argument.");
  return nativeOk(NUMBER_VAL((double)AS_STRING(args[0])->length));
}

// --- random() ----------------------------------------------------------------
// Returns a random float in [0, 1). Seeded once in initVM().
static NativeResult randomNative(int argCount, Value* args) {
  return nativeOk(NUMBER_VAL((double)rand() / ((double)RAND_MAX + 1.0)));
}

// --- random_int(lo, hi) ------------------------------------------------------
// Returns a random integer in the inclusive range [lo, hi].
static NativeResult randomIntNative(int argCount, Value* args) {
  if (!IS_NUMBER(args[0]) || !IS_NUMBER(args[1]))
    return nativeError("random_int() requires two number arguments.");
  int lo = (int)AS_NUMBER(args[0]);
  int hi = (int)AS_NUMBER(args[1]);
  if (lo > hi)
    return nativeError("random_int() requires lo <= hi.");
  return nativeOk(NUMBER_VAL((double)(lo + rand() % (hi - lo + 1))));
}

// =============================================================================

// Challenge 2: arity parameter added.
static void defineNative(const char* name, NativeFn function, int arity) {
  push(OBJ_VAL(copyString(name, (int)strlen(name))));
  push(OBJ_VAL(newNative(function, arity)));
  tableSet(&vm.globals, AS_STRING(vm.stack[0]), vm.stack[1]);
  pop();
  pop();
}

void initVM() {
  resetStack();
  initTable(&vm.globals);
  initTable(&vm.strings);

  srand((unsigned int)time(NULL));

  // *** CHALLENGE 4 *** Register all native functions.
  defineNative("clock",      clockNative,    0);
  defineNative("sqrt",       sqrtNative,     1);
  defineNative("abs",        absNative,      1);
  defineNative("floor",      floorNative,    1);
  defineNative("ceil",       ceilNative,     1);
  defineNative("min",        minNative,      2);
  defineNative("max",        maxNative,      2);
  defineNative("str_len",    strLenNative,   1);
  defineNative("random",     randomNative,   0);
  defineNative("random_int", randomIntNative, 2);
}

void freeVM() {
  freeTable(&vm.globals);
  freeTable(&vm.strings);
  freeObjects();
}

void push(Value value) {
  *vm.stackTop = value;
  vm.stackTop++;
}

Value pop() {
  vm.stackTop--;
  return *vm.stackTop;
}

static Value peek(int distance) {
  return vm.stackTop[-1 - distance];
}

static bool call(ObjFunction* function, int argCount) {
  if (argCount != function->arity) {
    runtimeError("Expected %d arguments but got %d.",
        function->arity, argCount);
    return false;
  }

  if (vm.frameCount == FRAMES_MAX) {
    runtimeError("Stack overflow.");
    return false;
  }

  CallFrame* frame = &vm.frames[vm.frameCount++];
  frame->function = function;
  frame->ip = function->chunk.code;
  frame->slots = vm.stackTop - argCount - 1;
  return true;
}

// Challenge 1: returns int so run() knows whether to LOAD_IP.
//   0 = new Lox frame pushed, caller must LOAD_IP
//   1 = native ran to completion, ip unchanged
//  -1 = runtime error
static int callValue(Value callee, int argCount) {
  if (IS_OBJ(callee)) {
    switch (OBJ_TYPE(callee)) {
      case OBJ_FUNCTION:
        return call(AS_FUNCTION(callee), argCount) ? 0 : -1;

      case OBJ_NATIVE: {
        // Challenge 2: validate arity before calling.
        ObjNative* native = AS_NATIVE_OBJ(callee);
        if (argCount != native->arity) {
          runtimeError("Expected %d arguments but got %d.",
              native->arity, argCount);
          return -1;
        }

        // Challenge 3: check NativeResult for errors.
        NativeResult result = native->function(argCount,
                                               vm.stackTop - argCount);
        if (!result.ok) {
          runtimeError("%s", result.as.error);
          return -1;
        }

        vm.stackTop -= argCount + 1;
        push(result.as.value);
        return 1;
      }

      default:
        break;
    }
  }
  runtimeError("Can only call functions and classes.");
  return -1;
}

static bool isFalsey(Value value) {
  return IS_NIL(value) || (IS_BOOL(value) && !AS_BOOL(value));
}

static void concatenate() {
  ObjString* b = AS_STRING(pop());
  ObjString* a = AS_STRING(pop());
  int length = a->length + b->length;
  char* chars = ALLOCATE(char, length + 1);
  memcpy(chars, a->chars, a->length);
  memcpy(chars + a->length, b->chars, b->length);
  chars[length] = '\0';
  ObjString* result = takeString(chars, length);
  push(OBJ_VAL(result));
}

static InterpretResult run() {
  CallFrame* frame = &vm.frames[vm.frameCount - 1];

  // Challenge 1: cache ip in a register-hinted local variable.
  register uint8_t* ip = frame->ip;

  #define READ_BYTE()     (*ip++)
  #define READ_SHORT() \
      (ip += 2, (uint16_t)((ip[-2] << 8) | ip[-1]))
  #define READ_CONSTANT() \
      (frame->function->chunk.constants.values[READ_BYTE()])
  #define READ_STRING() AS_STRING(READ_CONSTANT())
  #define BINARY_OP(valueType, op) \
      do { \
        if (!IS_NUMBER(peek(0)) || !IS_NUMBER(peek(1))) { \
          SAVE_IP(); \
          runtimeError("Operands must be numbers."); \
          return INTERPRET_RUNTIME_ERROR; \
        } \
        double b = AS_NUMBER(pop()); \
        double a = AS_NUMBER(pop()); \
        push(valueType(a op b)); \
      } while (false)

  for (;;) {
#ifdef DEBUG_TRACE_EXECUTION
    printf("          ");
    for (Value* slot = vm.stack; slot < vm.stackTop; slot++) {
      printf("[ ");
      printValue(*slot);
      printf(" ]");
    }
    printf("\n");
    disassembleInstruction(&frame->function->chunk,
        (int)(ip - frame->function->chunk.code));
#endif

    uint8_t instruction;
    switch (instruction = READ_BYTE()) {
      case OP_CONSTANT: {
        Value constant = READ_CONSTANT();
        push(constant);
        break;
      }
      case OP_NIL:   push(NIL_VAL); break;
      case OP_TRUE:  push(BOOL_VAL(true)); break;
      case OP_FALSE: push(BOOL_VAL(false)); break;
      case OP_POP:   pop(); break;
      case OP_GET_LOCAL: {
        uint8_t slot = READ_BYTE();
        push(frame->slots[slot]);
        break;
      }
      case OP_SET_LOCAL: {
        uint8_t slot = READ_BYTE();
        frame->slots[slot] = peek(0);
        break;
      }
      case OP_GET_GLOBAL: {
        ObjString* name = READ_STRING();
        Value value;
        if (!tableGet(&vm.globals, name, &value)) {
          SAVE_IP();
          runtimeError("Undefined variable '%s'.", name->chars);
          return INTERPRET_RUNTIME_ERROR;
        }
        push(value);
        break;
      }
      case OP_DEFINE_GLOBAL: {
        ObjString* name = READ_STRING();
        tableSet(&vm.globals, name, peek(0));
        pop();
        break;
      }
      case OP_SET_GLOBAL: {
        ObjString* name = READ_STRING();
        if (tableSet(&vm.globals, name, peek(0))) {
          tableDelete(&vm.globals, name);
          SAVE_IP();
          runtimeError("Undefined variable '%s'.", name->chars);
          return INTERPRET_RUNTIME_ERROR;
        }
        break;
      }
      case OP_EQUAL: {
        Value b = pop();
        Value a = pop();
        push(BOOL_VAL(valuesEqual(a, b)));
        break;
      }
      case OP_GREATER:  BINARY_OP(BOOL_VAL, >); break;
      case OP_LESS:     BINARY_OP(BOOL_VAL, <); break;
      case OP_ADD: {
        if (IS_STRING(peek(0)) && IS_STRING(peek(1))) {
          concatenate();
        } else if (IS_NUMBER(peek(0)) && IS_NUMBER(peek(1))) {
          double b = AS_NUMBER(pop());
          double a = AS_NUMBER(pop());
          push(NUMBER_VAL(a + b));
        } else {
          SAVE_IP();
          runtimeError("Operands must be two numbers or two strings.");
          return INTERPRET_RUNTIME_ERROR;
        }
        break;
      }
      case OP_SUBTRACT: BINARY_OP(NUMBER_VAL, -); break;
      case OP_MULTIPLY: BINARY_OP(NUMBER_VAL, *); break;
      case OP_DIVIDE:   BINARY_OP(NUMBER_VAL, /); break;
      case OP_NOT:
        push(BOOL_VAL(isFalsey(pop())));
        break;
      case OP_NEGATE:
        if (!IS_NUMBER(peek(0))) {
          SAVE_IP();
          runtimeError("Operand must be a number.");
          return INTERPRET_RUNTIME_ERROR;
        }
        push(NUMBER_VAL(-AS_NUMBER(pop())));
        break;
      case OP_PRINT: {
        printValue(pop());
        printf("\n");
        break;
      }
      case OP_JUMP: {
        uint16_t offset = READ_SHORT();
        ip += offset;
        break;
      }
      case OP_JUMP_IF_FALSE: {
        uint16_t offset = READ_SHORT();
        if (isFalsey(peek(0))) ip += offset;
        break;
      }
      case OP_LOOP: {
        uint16_t offset = READ_SHORT();
        ip -= offset;
        break;
      }
      case OP_CALL: {
        int argCount = READ_BYTE();
        SAVE_IP();
        int status = callValue(peek(argCount), argCount);
        if (status < 0) return INTERPRET_RUNTIME_ERROR;
        if (status == 0) {
          frame = &vm.frames[vm.frameCount - 1];
          LOAD_IP();
        }
        break;
      }
      case OP_RETURN: {
        Value result = pop();
        SAVE_IP();
        vm.frameCount--;
        if (vm.frameCount == 0) {
          pop();
          return INTERPRET_OK;
        }
        vm.stackTop = frame->slots;
        push(result);
        frame = &vm.frames[vm.frameCount - 1];
        LOAD_IP();
        break;
      }
    }
  }

  #undef READ_BYTE
  #undef READ_SHORT
  #undef READ_CONSTANT
  #undef READ_STRING
  #undef BINARY_OP
}

InterpretResult interpret(const char* source) {
  ObjFunction* function = compile(source);
  if (function == NULL) return INTERPRET_COMPILE_ERROR;

  push(OBJ_VAL(function));
  call(function, 0);

  return run();
}