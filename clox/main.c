#include "chunk.h"
#include "debug.h"

int main(void) {
  Chunk chunk;
  initChunk(&chunk);

  int constant = addConstant(&chunk, 1.2);
  writeChunk(&chunk, OP_CONSTANT, 123);
  writeChunk(&chunk, (uint8_t)constant, 123);

  constant = addConstant(&chunk, 3.4);
  writeChunk(&chunk, OP_CONSTANT, 123);
  writeChunk(&chunk, (uint8_t)constant, 123);

  writeChunk(&chunk, OP_RETURN, 124);

  disassembleChunk(&chunk, "test chunk");
  freeChunk(&chunk);
  return 0;
}