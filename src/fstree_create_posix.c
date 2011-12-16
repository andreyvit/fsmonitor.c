#include "fsmonitor_private.h"

#include <dirent.h>

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdarg.h>
#include <string.h>


static void fill_item(item_t *item, struct stat *st, char *name, int parent) {
  item->parent       = parent;
  item->name         = name;
  item->st_mode      = st->st_mode & S_IFMT;
  item->st_dev       = st->st_dev;
  item->st_ino       = st->st_ino;
  item->st_mtimespec = st->st_mtimespec;
  item->st_ctimespec = st->st_ctimespec;
  item->st_size      = st->st_size;
}


static int item_comparator(const void *a, const void *b) {
  item_t *aa = (item_t *)a, *bb = (item_t *)b;
  return strcmp(aa->name, bb->name);
}


fstree_t *fstree_create(const char *root_path, fsfilter_t *filter, fstree_t *previous) {
  size_t item_path_buf_size = 2;
  char  *item_path_buf = (char *)malloc(item_path_buf_size);
  size_t subitem_path_buf_size = 2;
  char  *subitem_path_buf = (char *)malloc(item_path_buf_size);

  struct stat st;
  struct dirent *dp;

  fstree_t *tree = (fstree_t *)malloc(sizeof(fstree_t));

  size_t items_size = 1; // a reasonable default
  if (previous && previous->count > items_size)
    items_size = previous->count;
  item_t *items = (item_t *)malloc(items_size * sizeof(item_t));
  tree->count = 0;

  // add root item
  if (0 != lstat(root_path, &st))
    goto fin;
  fill_item(&items[tree->count++], &st, strdup(""), 0);

  // process all directories, starting with the root
  for (int next = 0; next < tree->count; ++next) {
    struct item_t *item = &items[next];
    if (!FSTREE_ITEM_IS_DIR(item))
      continue;

    // used for sorting the newly added entries later
    int first = tree->count;

    // real path of the item
    const char *item_path = bufpathcat(&item_path_buf, &item_path_buf_size, root_path, item->name);

    // for each child
    DIR *dirp = opendir(item_path);
    if (!dirp)
      continue;
    while ((dp = readdir(dirp)) != NULL) {
      if (0 == strcmp(dp->d_name, ".") || 0 == strcmp(dp->d_name, ".."))
          continue;

      // process paths
      const char *subitem_name = dp->d_name;
      const char *subitem_path = bufstrcat(&subitem_path_buf, &subitem_path_buf_size, item_path, "/", subitem_name, NULL);

      if (0 == lstat(subitem_path, &st)) {
        // add the item to the tree
        const char *subitem_rel_path = bufpathcat(&subitem_path_buf, &subitem_path_buf_size, item->name, subitem_name);
        if (tree->count == items_size) {
          items_size *= 2;
          items = realloc(items, items_size * sizeof(item_t));
          item = &items[next];
        }
        fill_item(&items[tree->count++], &st, strdup(subitem_rel_path), next);
      }
    }
    closedir(dirp);

    // sort all the entries
    if (tree->count > first)
      qsort(items + first, tree->count - first, sizeof(item_t), item_comparator);
  }
fin:
  free(item_path_buf);
  free(subitem_path_buf);

  items = realloc(items, tree->count * sizeof(item_t));
  tree->items = items;
  return tree;
}
