Cross-platform File System Monitoring in C
==========================================

Like this:

    struct fstree_t *tree = fstree_create(path, NULL, NULL);
    fstree_dump(tree);

    ...

    struct fstree_t *tree2 = fstree_create(path, NULL, tree);
    fstree_dump(tree2);

    fsdiff_t *diff = fstree_diff(tree, tree2);
    fsdiff_dump(diff);

    fsdiff_free(diff);
    fstree_free(tree);
    fstree_free(tree2);


Building on Mac
---------------

    make


Building on Windows
-------------------

Compile with /TP (as C++ code) because we're using C99 features. Also use /Iheaders. The demo project already has these settings.


Running the demo
----------------

Pass some existing directory as an argument.
