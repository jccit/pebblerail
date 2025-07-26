#pragma once

#include <pebble.h>

void window_manager_push_window(Window *window);
void window_manager_destroy_window(Window *window);

// Removes the oldest window from the stack to free up memory
void window_manager_free_oldest();

// Ensure we have enough space for the requested window
// This will free old windows if necessary
void *wm_alloc(size_t size);