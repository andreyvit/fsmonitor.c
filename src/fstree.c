
#include "fsmonitor_private.h"

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

const char *FSTREE_ITEM_TYPE_NAMES[] = { "file", "dir", "link", "other" };

const char *fstree_item_type_name(fstree_item_type type) {
    assert(type >= 0);
    assert(type <= fstree_item_type_other);
    return FSTREE_ITEM_TYPE_NAMES[type];
}

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

int fstree_count(fstree_t *tree) {
    return tree->count;
}

#define WIN_100NS_TICKS_PER_SEC 10000000
#define WIN_SEC_EPOCH_DIFFERENCE 11644473600LL
const char *fstree_get(fstree_t *tree, int index, int *parent_index, fstree_item_type *type, long *size, long *time_sec, long *time_nsec) {
    item_t *item = &tree->items[index];
    if (parent_index)
        *parent_index = item->parent;
    if (type)
        *type = FSTREE_ITEM_TYPE(item);
#ifdef WIN32
    if (size)
        *size = (long) (item->size > LONG_MAX ? LONG_MAX : item->size);

    ULONGLONG win_time_100ns = ((ULONGLONG)item->write_time.dwHighDateTime << 32) | (ULONGLONG)item->write_time.dwLowDateTime;
    ULONGLONG win_time_sec   = (win_time_100ns / WIN_100NS_TICKS_PER_SEC) - WIN_SEC_EPOCH_DIFFERENCE;
    ULONGLONG win_time_nsec  = (win_time_100ns % WIN_100NS_TICKS_PER_SEC) * 100;

    if (time_sec)
        *time_sec = (long) win_time_sec;
    if (time_nsec)
        *time_nsec = (long) win_time_nsec;
#else
    if (size)
        *size = (long) (item->st_size > LONG_MAX ? LONG_MAX : item->st_size);
    if (time_sec)
        *time_sec = (long) item->st_mtimespec.tv_sec;
    if (time_nsec)
        *time_nsec = (long) item->st_mtimespec.tv_nsec;
#endif
    return item->name;
}
