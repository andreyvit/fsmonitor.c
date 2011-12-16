#ifndef FSMONITOR_PRIVATE_H_INCLUDED
#define FSMONITOR_PRIVATE_H_INCLUDED

#include "fsmonitor.h"

#ifdef _WIN32
#include <windows.h>
#pragma warning(disable: 4996)
#else
#include <stdbool.h>
#endif

#include <sys/stat.h>

/**************************************************************/

#define FSMONITOR_DEBUG 1

/**************************************************************/

struct fsdiff_t {
  char **paths;
  int count;
};

/**************************************************************/

typedef struct item_t {
  char *name;
  int parent;
#ifdef _WIN32
  DWORD attr;
  FILETIME write_time;
#else
  mode_t st_mode;
  dev_t st_dev;
  ino_t st_ino;
  struct timespec st_mtimespec;
  struct timespec st_ctimespec;
  off_t st_size;
#endif
} item_t;

struct fstree_t {
  item_t *items;
  int count;
};

#ifdef _WIN32
#define FSTREE_ITEM_IS_DIR(item) ((item)->attr & FILE_ATTRIBUTE_DIRECTORY)
#define FSTREE_ITEM_IS_REG(item) (!FSTREE_ITEM_IS_DIR(item))
#else
#define FSTREE_ITEM_IS_DIR(item) ((item)->st_mode == S_IFDIR)
#define FSTREE_ITEM_IS_REG(item) ((item)->st_mode == S_IFREG)
#endif


/**************************************************************/

#include <stdarg.h>
#include <string.h>
#include <stdlib.h>

#ifdef _WIN32
static char *stpcpy(char *dest, const char *source) {
  strcpy(dest, source);
  return dest + strlen(dest);
}
#endif

static char *bufstrcat(char **buf, size_t *buf_size, ...) {
  va_list va;
  const char *item;
  size_t size = 1;

  va_start(va, buf_size);
  while ((item = va_arg(va, const char *)) != NULL) {
    size += strlen(item);
  }
  va_end(va);

  if (*buf_size < size) {
    *buf_size = (size > *buf_size * 2 ? size : *buf_size * 2);
    *buf = (char *)realloc(*buf, *buf_size);
  }

  char *ptr = *buf;
  *ptr = 0;

  va_start(va, buf_size);
  while ((item = va_arg(va, const char *)) != NULL) {
    ptr = stpcpy(ptr, item);
  }
  va_end(va);

  return *buf;
}

static const char *bufpathcat(char **buf, size_t *buf_size, const char *a, const char *b) {
  if (*a && *b)
    return bufstrcat(buf, buf_size, a, "/", b, NULL);
  else if (*a)
    return a;
  else
    return b;
}

#endif
