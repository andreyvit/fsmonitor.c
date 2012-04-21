#include "fsmonitor_private.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>



static void fslistener_FSMonitorEventStreamCallback(ConstFSEventStreamRef streamRef, fslistener_t *listener, size_t numEvents, char *eventPaths[], const FSEventStreamEventFlags eventFlags[], const FSEventStreamEventId eventIds[]);



fslistener_t *fslistener_create(const char *path, fslistener_callback_t callback, void *data) {
  fslistener_t *listener = (fslistener_t *) malloc(sizeof(fslistener_t));

  listener->path = strdup(path);
  listener->callback = callback;
  listener->callback_data = data;

  FSEventStreamContext context;
  context.version = 0;
  context.info = listener;
  context.retain = NULL;
  context.release = NULL;
  context.copyDescription = NULL;

  CFStringRef pathRef = CFStringCreateWithCString(NULL, path, kCFStringEncodingUTF8);
  CFArrayRef paths = CFArrayCreate(NULL, (const void **)&pathRef, 1, &kCFTypeArrayCallBacks);
  CFRelease(pathRef);

  listener->streamRef = FSEventStreamCreate(NULL, (FSEventStreamCallback)fslistener_FSMonitorEventStreamCallback, &context, paths, kFSEventStreamEventIdSinceNow, 0.05, 0);
  assert(listener->streamRef && "FSEventStreamCreate failed");

  CFRelease(paths);

  FSEventStreamScheduleWithRunLoop(listener->streamRef, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
  bool result = FSEventStreamStart(listener->streamRef);
  assert(result && "FSEventStreamStart failed");

  listener->callback(listener->path, fslistener_hint_startup, listener->callback_data);

  return listener;
}

void fslistener_free(fslistener_t *listener) {
  FSEventStreamStop(listener->streamRef);
  FSEventStreamInvalidate(listener->streamRef);
  FSEventStreamRelease(listener->streamRef);
  listener->streamRef = nil;

  listener->callback(listener->path, fslistener_hint_shutdown, listener->callback_data);

  free(listener->path);
  free(listener);
}

static void fslistener_FSMonitorEventStreamCallback(ConstFSEventStreamRef streamRef, fslistener_t *listener, size_t numEvents, char *eventPaths[], const FSEventStreamEventFlags eventFlags[], const FSEventStreamEventId eventIds[]) {
  for (size_t i = 0; i < numEvents; ++i) {
    // TODO: check eventFlags[i] to give a better hint
    listener->callback(eventPaths[i], fslistener_hint_dir_deep, listener->callback_data);
  }
}
