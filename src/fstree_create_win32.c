#include "fsmonitor_private.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdarg.h>
#include <string.h>


static void fill_item(item_t *item, WIN32_FIND_DATA *st, char *name, int parent) {
  item->parent     = parent;
  item->name       = name;
  item->attr       = st->dwFileAttributes;
  item->write_time = st->ftLastWriteTime;
  item->size       = ((ULONGLONG)st->nFileSizeHigh << 32) | ((ULONGLONG)st->nFileSizeLow);
}


static int item_comparator(const void *a, const void *b) {
  item_t *aa = (item_t *)a, *bb = (item_t *)b;
  return strcmp(aa->name, bb->name);
}


static char *w2u(WCHAR *string) {
  DWORD cb = WideCharToMultiByte(CP_UTF8, 0, string, -1, NULL, 0, NULL, NULL);
  char *result = (char *) malloc(cb);
  WideCharToMultiByte(CP_UTF8, 0, string, -1, result, cb, NULL, NULL);
  return result;
}


fstree_t *fstree_create(const char *root_path, fsfilter_t *filter, fstree_t *previous) {
  size_t item_path_buf_size = 2;
  char  *item_path_buf = (char *)malloc(item_path_buf_size);
  size_t subitem_path_buf_size = 2;
  char  *subitem_path_buf = (char *)malloc(item_path_buf_size);


  WIN32_FIND_DATA st;
  HANDLE hFind;
  WCHAR buf[MAX_PATH];

  fstree_t *tree = (fstree_t *)malloc(sizeof(fstree_t));

  int items_size = 1; // a reasonable default
  if (previous && previous->count > items_size)
    items_size = previous->count;
  item_t *items = (item_t *)malloc(items_size * sizeof(item_t));
  tree->count = 0;

  // add root item
  MultiByteToWideChar(CP_UTF8, 0, root_path, -1, buf, MAX_PATH);
  hFind = FindFirstFile(buf, &st);
  if (hFind == INVALID_HANDLE_VALUE)
    goto fin;
  FindClose(hFind);
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

    MultiByteToWideChar(CP_UTF8, 0, item_path, -1, buf, MAX_PATH);
    wcscat(buf, L"/*.*");
    hFind = FindFirstFile(buf, &st);
    if (hFind == INVALID_HANDLE_VALUE)
      continue;
    do {
      if (0 == wcscmp(st.cFileName, L".") || 0 == wcscmp(st.cFileName, L".."))
          continue;
      char *subitem_name = w2u(st.cFileName);
      const char *subitem_path = bufstrcat(&subitem_path_buf, &subitem_path_buf_size, item_path, "/", subitem_name, NULL);

      const char *subitem_rel_path = bufpathcat(&subitem_path_buf, &subitem_path_buf_size, item->name, subitem_name);
      if (tree->count == items_size) {
        items_size *= 2;
        items = (item_t *) realloc(items, items_size * sizeof(item_t));
        item = &items[next];
      }
      fill_item(&items[tree->count++], &st, strdup(subitem_rel_path), next);

      free(subitem_name);
    } while (FindNextFile(hFind, &st));
    FindClose(hFind);

    // sort all the entries
    if (tree->count > first)
      qsort(items + first, tree->count - first, sizeof(item_t), item_comparator);
  }
fin:
  free(item_path_buf);
  free(subitem_path_buf);

  items = (item_t *) realloc(items, tree->count * sizeof(item_t));
  tree->items = items;
  return tree;
}
