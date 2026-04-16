#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "table.h"
#include "value.h"

int main() {
  Table table;
  initTable(&table);

  srand(42);
  clock_t start = clock();

  for (int i = 0; i < 100000; i++) {
    int key = rand();
    tableSet(&table, NUMBER_VAL(key), NUMBER_VAL(key));
  }

  for (int i = 0; i < 100000; i++) {
    int key = rand();
    Value value;
    tableGet(&table, NUMBER_VAL(key), &value);
  }

  clock_t end = clock();
  printf("Random test time: %f\n", (double)(end - start) / CLOCKS_PER_SEC);

  freeTable(&table);
  return 0;
}