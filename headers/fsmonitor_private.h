#ifndef FSMONITOR_PRIVATE_H_INCLUDED
#define FSMONITOR_PRIVATE_H_INCLUDED

#include "fsmonitor.h"

#ifdef _WIN32
#include <windows.h>
#pragma warning(disable: 4996)
#else
#include <stdbool.h>
#include <CoreServices/CoreServices.h>
#endif

#include <sys/stat.h>

#ifdef __APPLE__
#pragma clang diagnostic ignored "-Wunused-function"
#endif

/**************************************************************/

#define FSMONITOR_DEBUG 1

// 0 off, 1 log changed files, 2 log changed and unchanged files
#define FSMONITOR_DIFF_DEBUG 0
// #define FSMONITOR_DIFF_DEBUG 1
// #define FSMONITOR_DIFF_DEBUG 2

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
  ULONGLONG size;
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
  size_t count;
};

#ifdef _WIN32
#define FSTREE_ITEM_IS_DIR(item) ((item)->attr & FILE_ATTRIBUTE_DIRECTORY)
#define FSTREE_ITEM_IS_REG(item) (!FSTREE_ITEM_IS_DIR(item))
#define FSTREE_ITEM_IS_LNK(item) (0)
#define FSTREE_ITEM_TYPE(item) (FSTREE_ITEM_IS_DIR(item) ? fstree_item_type_dir : fstree_item_type_file)
#else
#define FSTREE_ITEM_IS_DIR(item) ((item)->st_mode == S_IFDIR)
#define FSTREE_ITEM_IS_REG(item) ((item)->st_mode == S_IFREG)
#define FSTREE_ITEM_IS_LNK(item) ((item)->st_mode == S_IFLNK)
#define FSTREE_ITEM_TYPE(item) (FSTREE_ITEM_IS_REG(item) ? fstree_item_type_file : (FSTREE_ITEM_IS_DIR(item) ? fstree_item_type_dir : (FSTREE_ITEM_IS_LNK(item) ? fstree_item_type_link : fstree_item_type_other)))
#endif

/**************************************************************/

struct fslistener_t {
  char *path;
  fslistener_callback_t callback;
  void *callback_data;
#ifdef _WIN32
  HANDLE hThread;
  HANDLE hShutDownEvent;
#else
  FSEventStreamRef streamRef;
#endif
};

/**************************************************************/

struct fsmonitor_t {
  char *path;
  fstree_t *tree;
  fslistener_t *listener;
  fsmonitor_callback_t callback;
  void *callback_data;
};

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
