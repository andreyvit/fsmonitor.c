
#include "fsmonitor_private.h"

#include <stdlib.h>
#include <stdio.h>

void fstree_free(fstree_t *tree) {
  for (item_t *cur = tree->items, *end = cur + tree->count; cur < end; ++cur) {
    free(cur->name);
  }
  free(tree->items);
  free(tree);
}

void fstree_dump(fstree_t *tree) {
  printf("Tree with %lu items:\n", tree->count);
  for (size_t i = 0; i < tree->count; ++i) {
    printf(" %2lu) %s\n", i, tree->items[i].name);
  }
}
