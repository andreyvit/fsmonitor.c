
.PHONY: all

.DEFAULT: all

all: bin/fsmonitor_demo

SRC=src/fsdiff.c src/fstree.c src/fstree_create_posix.c src/fstree_diff.c src/fslistener_osx.c src/fsmonitor.c demo/fsmonitor_demo.c

bin/fsmonitor_demo: $(SRC) headers/fsmonitor.h headers/fsmonitor_private.h
	mkdir -p `dirname $@`
	gcc -I./headers -framework CoreServices -framework CoreFoundation -std=c99 -g -O0 -o $@ $(SRC)
