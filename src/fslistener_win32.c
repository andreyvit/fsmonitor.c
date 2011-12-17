#include "fsmonitor_private.h"

#include <malloc.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

#include <windows.h>
#include <process.h>


static unsigned __stdcall fslistener_thread(void *listener);


fslistener_t *fslistener_create(const char *path, fslistener_callback_t callback, void *data) {
  fslistener_t *listener = (fslistener_t *) malloc(sizeof(fslistener_t));
  listener->path = strdup(path);
  listener->callback = callback;
  listener->callback_data = data;
  listener->hShutDownEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
  listener->hThread = (HANDLE) _beginthreadex(NULL, 0, fslistener_thread, listener, 0, NULL);
  assert(listener->hThread);
  return listener;
}

void fslistener_free(fslistener_t *listener) {
  SetEvent(listener->hShutDownEvent);
  DWORD dwWaitResult = WaitForSingleObject(listener->hThread, INFINITE);
  assert(dwWaitResult != WAIT_FAILED);
  CloseHandle(listener->hShutDownEvent);
  CloseHandle(listener->hThread);
  free(listener->path);
  free(listener);
}

static unsigned __stdcall fslistener_thread(void *param) {
  fslistener_t *listener = (fslistener_t *)param;
  WCHAR buf[MAX_PATH];
  MultiByteToWideChar(CP_UTF8, 0, listener->path, -1, buf, MAX_PATH);

  HANDLE hChange = FindFirstChangeNotification(buf, TRUE, FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME | FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_SIZE | FILE_NOTIFY_CHANGE_ATTRIBUTES);
  assert(hChange != INVALID_HANDLE_VALUE);
  printf("Listening to changes in %s\n", listener->path);

  HANDLE hWaitObjects[] = { hChange, listener->hShutDownEvent };

  listener->callback(listener->path, fslistener_hint_startup, listener->callback_data);
  while (TRUE) {
    DWORD status = WaitForMultipleObjects(2, hWaitObjects, FALSE, INFINITE);
    if (status == WAIT_OBJECT_0) {
      printf("Detected change in %s\n", listener->path);
      listener->callback(listener->path, fslistener_hint_dir_deep, listener->callback_data);
      DWORD result = FindNextChangeNotification(hChange);
      assert(result);
    } else if (status == WAIT_OBJECT_0 + 1) {
      break; // shutdown signalled
    } else {
      assert(!"WaitForMultipleObjects returned error");
    }
  }
  listener->callback(listener->path, fslistener_hint_shutdown, listener->callback_data);
  return 0;
}
