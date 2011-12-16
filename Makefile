
.PHONY: all

.DEFAULT: all

all: bin/fsmonitor_demo

SRC=src/fsdiff.c src/fstree.c src/fstree_create_posix.c src/fstree_diff.c demo/fsmonitor_demo.c

bin/fsmonitor_demo: $(SRC) headers/fsmonitor.h headers/fsmonitor_private.h
	mkdir -p `dirname $@`
	gcc -I./headers -std=c99 -o $@ $(SRC)
