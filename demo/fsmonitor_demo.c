
#include "fsmonitor.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef _WIN32
#include <unistd.h>
#endif

int main(int argc, char **argv) {
    if (argc != 2) {
        printf("usage: fstree_test /some/path\n");
        return 1;
    }
    const char *path = argv[1];

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
