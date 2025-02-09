#pragma once

#include <pebble.h>

GRect bounds_with_status_bar(Window *window);
TextLayer *create_error_layer(Window *window, char *message);