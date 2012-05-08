#include "fsmonitor_private.h"

static void fsmonitor_listener_callback(const char *path, fslistener_hint hint, void *data);

fsmonitor_t *fsmonitor_create(const char *path, fsfilter_t *filter, fsmonitor_callback_t callback, void *data) {
    fsmonitor_t *monitor = (fsmonitor_t *) malloc(sizeof(fsmonitor_t));
    monitor->path = strdup(path);
    monitor->tree = NULL;
    monitor->callback = callback;
    monitor->callback_data = data;
    monitor->listener = fslistener_create(path, fsmonitor_listener_callback, monitor);
    return monitor;
}

void fsmonitor_free(fsmonitor_t *monitor) {
    fslistener_free(monitor->listener);
    free(monitor->path);
    free(monitor);
}

static void fsmonitor_listener_callback(const char *path, fslistener_hint hint, void *data) {
    fsmonitor_t *monitor = (fsmonitor_t *)data;
    if (hint == fslistener_hint_startup) {
        monitor->tree = fstree_create(monitor->path, NULL, NULL);
        monitor->callback(NULL, monitor->tree, monitor->callback_data);
    } else if (hint == fslistener_hint_shutdown) {
        fstree_free(monitor->tree);
        monitor->tree = NULL;
    } else {
        fstree_t *previous = monitor->tree;
        fstree_t *current  = fstree_create(monitor->path, NULL, previous);
        monitor->tree = current;
        fsdiff_t *diff = fstree_diff(previous, current);
        fstree_free(previous);
        if (fsdiff_count(diff) > 0) {
            monitor->callback(diff, monitor->tree, monitor->callback_data);
        }
    }
}
