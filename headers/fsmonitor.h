#ifndef FSMONITOR_H_INCLUDED
#define FSMONITOR_H_INCLUDED

/**************************************************************/

typedef struct fsfilter_t fsfilter_t;

fsfilter_t *fsfilter_create(const char **included_extensions, const char **excluded_paths);
void fsfilter_free(fsfilter_t *filter);

/**************************************************************/

typedef struct fsdiff_t fsdiff_t;

void fsdiff_free(fsdiff_t *diff);

int fsdiff_count(fsdiff_t *diff);
const char *fsdiff_get(fsdiff_t *diff, int index);
void fsdiff_dump(fsdiff_t *diff);

/**************************************************************/

typedef enum {
    fstree_item_type_file,
    fstree_item_type_dir,
    fstree_item_type_link,
    fstree_item_type_other,
} fstree_item_type;

const char *fstree_item_type_name(fstree_item_type type);

/**************************************************************/

typedef struct fstree_t fstree_t;

fstree_t *fstree_create(const char *path, fsfilter_t *filter, fstree_t *previous);
void fstree_free(fstree_t *tree);

int fstree_count(fstree_t *tree);
const char *fstree_get(fstree_t *tree, int index, int *parent_index, fstree_item_type *type, long *size, long *time_sec, long *time_nsec);

void fstree_dump(fstree_t *tree);

/**************************************************************/

fsdiff_t *fstree_diff(fstree_t *previous, fstree_t *current);

/**************************************************************/

typedef struct fslistener_t fslistener_t;

typedef enum {
  fslistener_hint_startup     = 10,
  fslistener_hint_shutdown    = 11,
  fslistener_hint_file        = 1,
  fslistener_hint_dir_shallow = 2,
  fslistener_hint_dir_deep    = 3,
} fslistener_hint;

// can called on any thread
// diff ownership is transferred to the callback, call fsdiff_free(diff) when done
typedef void (*fslistener_callback_t)(const char *path, fslistener_hint hint, void *data);

fslistener_t *fslistener_create(const char *path, fslistener_callback_t callback, void *data);
void fslistener_free(fslistener_t *listener);

/**************************************************************/

typedef struct fsmonitor_t fsmonitor_t;

// diff ownership is transferred to the callback, call fsdiff_free(diff) when done
// tree ownership is NOT transferred to the callback, and the tree MUST NOT be accessed after returning from the callback
typedef void (*fsmonitor_callback_t)(fsdiff_t *diff, fstree_t *tree, void *data);

fsmonitor_t *fsmonitor_create(const char *path, fsfilter_t *filter, fsmonitor_callback_t callback, void *data);
void fsmonitor_free(fsmonitor_t *monitor);

#endif
