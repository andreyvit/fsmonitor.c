#include "fsmonitor_private.h"

#include <malloc.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

#include <windows.h>
#include <process.h>


static void fslistener_thread(fslistener_t *listener);


fslistener_t *fslistener_create(const char *path, fslistener_callback_t callback, void *data) {
  fslistener_t *listener = (fslistener_t *) malloc(sizeof(fslistener_t));
  listener->path = strdup(path);
  listener->callback = callback;
  listener->callback_data = data;
  listener->hThread = (HANDLE) _beginthread((void(*)(void*))fslistener_thread, 0, listener);
  return listener;
}

void fslistener_free(fslistener_t *listener) {
  free(listener->path);
  free(listener);
}

static void fslistener_thread(fslistener_t *listener) {
  WCHAR buf[MAX_PATH];
  MultiByteToWideChar(CP_UTF8, 0, listener->path, -1, buf, MAX_PATH);

  HANDLE hChange = FindFirstChangeNotification(buf, TRUE, FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME | FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_SIZE | FILE_NOTIFY_CHANGE_ATTRIBUTES);
  assert(hChange != INVALID_HANDLE_VALUE);
  printf("Listening to changes in %s\n", listener->path);

  listener->callback(listener->path, fslistener_hint_startup, listener->callback_data);
  while (TRUE) {
    DWORD status = WaitForMultipleObjects(1, &hChange, FALSE, INFINITE);
    if (status == WAIT_OBJECT_0) {
      printf("Detected change in %s\n", listener->path);
      listener->callback(listener->path, fslistener_hint_dir_deep, listener->callback_data);
      DWORD result = FindNextChangeNotification(hChange);
      assert(result);
    } else {
      assert(!"WaitForMultipleObjects returned error");
    }
  }
  listener->callback(listener->path, fslistener_hint_shutdown, listener->callback_data);
}
