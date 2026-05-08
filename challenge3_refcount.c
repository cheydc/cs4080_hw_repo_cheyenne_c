// ============================================================
// CHALLENGE 3: Reference Counting (augmenting mark-sweep)
//
// Reference counting frees objects immediately when their ref count
// drops to zero, rather than waiting for the next GC cycle.
// It cannot handle cycles, so we keep mark-sweep for that case.
// ============================================================

// Add to struct Obj in object.h:
//   size_t refCount;

// Add to memory.h:
//   void incRef(Obj* object);
//   void decRef(Obj* object);

// Increment reference count. Call whenever a new reference to an object
// is created (stored in a variable, pushed onto stack, etc.)
void incRef(Obj* object) {
    if (object == NULL) return;
    object->refCount++;
}

// Decrement reference count. When it hits zero, recursively decrement
// any objects this one references, then free it.
// NOTE: Cannot free cycles -- mark-sweep handles those.
void decRef(Obj* object) {
    if (object == NULL) return;
    if (--object->refCount > 0) return;

    switch (object->type) {
        case OBJ_CLOSURE: {
            ObjClosure* closure = (ObjClosure*)object;
            decRef((Obj*)closure->function);
            for (int i = 0; i < closure->upvalueCount; i++) {
                decRef((Obj*)closure->upvalues[i]);
            }
            break;
        }
        case OBJ_FUNCTION: {
            ObjFunction* function = (ObjFunction*)object;
            decRef((Obj*)function->name);
            for (int i = 0; i < function->chunk.constants.count; i++) {
                Value v = function->chunk.constants.values[i];
                if (IS_OBJ(v)) decRef(AS_OBJ(v));
            }
            break;
        }
        case OBJ_UPVALUE:
            if (IS_OBJ(((ObjUpvalue*)object)->closed))
                decRef(AS_OBJ(((ObjUpvalue*)object)->closed));
            break;
        case OBJ_STRING:
        case OBJ_NATIVE:
            break;
        default:
            break;
    }

    freeObject(object);
}

// Key integration points in vm.c:
//
// push() -- incRef when a value lands on the stack
// pop()  -- caller must decRef when it truly drops the reference
//
// OP_SET_GLOBAL / OP_SET_LOCAL / OP_SET_UPVALUE:
//   decRef old value, incRef new value
//
// Limitation: cycles are never freed by ref counting alone.
// Python solves this by running a cyclic garbage detector alongside
// reference counting as a fallback.