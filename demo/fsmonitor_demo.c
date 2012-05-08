
#include "fsmonitor.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#pragma warning(disable: 4996)
#else
#include <unistd.h>
#include <CoreFoundation/CoreFoundation.h>
#endif

void fsmonitor_callback(fsdiff_t *diff, fstree_t *tree, void *data) {
    if (!diff) return;
    fsdiff_dump(diff);
}

int main(int argc, char **argv) {
    if (argc < 2 || argc > 3) {
        printf("usage: fsmonitor_demo /some/path\n");
        printf("usage: fsmonitor_demo /some/path --selftest\n");
        return 1;
    }
    const char *path = argv[1];

    if (argc == 3 && 0 == strcmp(argv[2], "--selftest")) {
        char buf[1024];
        sprintf(buf, "%s/foobarboz.txt", path);

        unlink(buf);

        struct fstree_t *tree = fstree_create(path, NULL, NULL);
        fstree_dump(tree);

        fclose(fopen(buf, "w"));

        struct fstree_t *tree2 = fstree_create(path, NULL, tree);
        fstree_dump(tree2);

        fsdiff_t *diff = fstree_diff(tree, tree2);
        fsdiff_dump(diff);

        fsdiff_free(diff);
        fstree_free(tree);
        fstree_free(tree2);
        return 0;
    }

    fsmonitor_t *monitor = fsmonitor_create(path, NULL, fsmonitor_callback, NULL);
    printf("Monitoring %s\n", path);

#ifdef __APPLE__
    CFRunLoopRun();
    // CFRunLoopRunInMode(kCFRunLoopDefaultMode, 5, false);
#else
    int dummy;
    scanf("%d", &dummy);
#endif
    fsmonitor_free(monitor);

    return 0;
}
