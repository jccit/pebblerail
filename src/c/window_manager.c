#include "window_manager.h"

#include "utils.h"

#ifdef PBL_PLATFORM_APLITE
#define MEMORY_SAFETY_MARGIN 1600
#define MAX_WINDOWS 4
#else
#define MEMORY_SAFETY_MARGIN 2048
#define MAX_WINDOWS 8
#endif

static Window *s_windows[MAX_WINDOWS];
static uint8_t s_window_count = 0;

void window_manager_push_window(Window *window) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "WM: Pushing window");

  if (s_window_count >= MAX_WINDOWS) {
    window_manager_free_oldest();
  }

  s_windows[s_window_count++] = window;

  APP_LOG(APP_LOG_LEVEL_DEBUG_VERBOSE, "WM: New window count: %d", s_window_count);

  check_free_memory();

  window_stack_push(window, true);
}

void window_manager_destroy_window(Window *window) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "WM: Destroying window");

  for (uint8_t i = 0; i < s_window_count; ++i) {
    if (s_windows[i] == window) {
      // shift array to left after removing
      for (uint8_t j = i; j < s_window_count - 1; ++j) {
        s_windows[j] = s_windows[j + 1];
      }
      s_window_count--;
      break;
    }
  }

  APP_LOG(APP_LOG_LEVEL_DEBUG_VERBOSE, "WM: Window count after free: %d", s_window_count);

  window_stack_remove(window, false);
}

void window_manager_free_oldest() {
  if (s_window_count <= 1) {
    APP_LOG(APP_LOG_LEVEL_WARNING, "WM: Not enough windows to free");
    return;
  }

  APP_LOG(APP_LOG_LEVEL_DEBUG, "WM: Freeing oldest window");
  window_manager_destroy_window(s_windows[0]);
}

void *wm_alloc(size_t requested_size) {
  APP_LOG(APP_LOG_LEVEL_DEBUG_VERBOSE, "WM: Allocating %d bytes", requested_size);

  size_t needed_memory = requested_size + MEMORY_SAFETY_MARGIN;
  size_t free_memory = check_free_memory();

  if (free_memory < needed_memory) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "WM: Not enough free memory, freeing old windows");
    window_manager_free_oldest();
  }

  return malloc(requested_size);
}