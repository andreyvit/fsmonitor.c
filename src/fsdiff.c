#include "fsmonitor_private.h"

#include <stdlib.h>
#include <stdio.h>

void fsdiff_free(fsdiff_t *diff) {
  char **end = diff->paths + diff->count;
  for (char **cur = diff->paths; cur < end; ++cur)
    free(*cur);
  free(diff->paths);
  free(diff);
}

int fsdiff_count(fsdiff_t *diff) {
  return diff->count;
}

const char *fsdiff_get(fsdiff_t *diff, int index) {
  return diff->paths[index];
}

void fsdiff_dump(fsdiff_t *diff) {
  printf("Diff with %d items:\n", diff->count);
  for (int i = 0; i < diff->count; ++i) {
    printf(" %2d) %s\n", i, diff->paths[i]);
  }
}
