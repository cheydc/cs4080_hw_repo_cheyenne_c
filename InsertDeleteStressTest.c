#include <stdio.h>
#include <time.h>
#include "table.h"
#include "value.h"

int main() {
  Table table;
  initTable(&table);

  clock_t start = clock();

  for (int i = 0; i < 50000; i++) {
    tableSet(&table, NUMBER_VAL(i), NUMBER_VAL(i));
  }

  for (int i = 0; i < 50000; i++) {
    tableDelete(&table, NUMBER_VAL(i));
  }

  for (int i = 0; i < 50000; i++) {
    tableSet(&table, NUMBER_VAL(i), NUMBER_VAL(i));
  }

  clock_t end = clock();
  printf("Delete stress test time: %f\n", (double)(end - start) / CLOCKS_PER_SEC);

  freeTable(&table);
  return 0;
}