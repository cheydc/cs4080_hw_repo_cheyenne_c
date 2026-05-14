# CS4080 - Crafting Interpreters Homework

## Branch Structure

**`chapters24-30`** (this branch): Contains all challenge problem implementations
for Chapters 24 through 30. This is the primary branch for the majority of the
homework.

**`up-to-chapter23`**: Contains the original clox implementation through Chapter 23.

## Why Two Branches

I created the `chapters24-30` branch early as a head start for our group
presentation on Chapters 24-25. By the time the course caught up to those
chapters, I had already completed the challenge problems there and continued
building on that branch rather than merging back into main. This left a gap in
the main branch. The two branches together represent the complete set of work.

## Note on Commits

All code changes were made directly through the terminal during development
sessions. As a result, some commit timestamps may appear old or infrequent and
do not accurately reflect when the work was completed. The actual implementations
are in the source files.

## Challenge Problems Completed

### Chapter 24 - Calls and Functions
- Challenge 1: Register-cached ip for ~19% speedup on fibonacci benchmark
- Challenge 2: Native function arity checking
- Challenge 3: NativeResult struct for error signaling from native functions
- Challenge 4: Additional native functions (sqrt, abs, floor, ceil, min, max, str_len, random, random_int, print_err, type_of)

### Chapter 25 - Closures
- Challenge 1: Skip ObjClosure wrapper for functions with no upvalues
- Challenge 2: Per-iteration loop variable for closures in for loops
- Challenge 3: Vector objects implemented via closures (vector.lox)

### Chapter 26 - Garbage Collection
- Challenge 1: Measured sizeof(Obj) = 16 bytes, analyzed compact representations
- Challenge 2: Generation flip to eliminate per-object mark clearing in sweep()
- Challenge 3: Reference counting implementation sketch (challenge3_refcount.c)

### Chapter 27 - Classes and Instances
- Challenge 1: Missing fields return nil instead of runtime error
- Challenge 2: Dynamic field access via getField() and setField() native functions
- Challenge 3: deleteField() native function using tableDelete()
- Challenge 4: Written analysis of inline caching and hidden classes

### Chapter 28 - Methods and Initializers
- Challenge 1: Cache init() method directly on ObjClass for faster instantiation
- Challenge 2: Written analysis of inline caching for method calls
- Challenge 3: Written analysis of field/method shadowing tradeoffs

### Chapter 29 - Superclasses
- Challenge 1: Runtime field ownership tracking to detect inheritance conflicts
- Challenge 2: Written analysis of open class modification and method caching
- Challenge 3: Full BETA language semantics with inner() keyword

### Chapter 30 - Optimization
- Challenge 1: Hash table modulo to bitwise AND optimization in table.c
- Challenge 2: Short String Optimization (SSO) storing strings <= 7 chars inline
- Challenge 3: Personal reflection on learning experience
