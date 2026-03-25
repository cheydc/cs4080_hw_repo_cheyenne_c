#include "chunk.h"
#include "debug.h"

int main(void) {
  Chunk chunk;
  initChunk(&chunk);

  writeConstant(&chunk, 1.2, 123);
  writeConstant(&chunk, 3.4, 123);
  writeChunk(&chunk, OP_RETURN, 124);

  disassembleChunk(&chunk, "test chunk");
  freeChunk(&chunk);
  return 0;
}