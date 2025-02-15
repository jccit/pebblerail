#pragma once

#include <pebble.h>

GRect bounds_with_status_bar(Window *window);
GRect bounds_with_status_bar_no_padding(Window *window);
TextLayer *create_error_layer(Window *window, char *message);

GSize graphics_draw_text_get_size(GContext *ctx, char *text, GFont font, GRect bounds, GTextOverflowMode overflow_mode, GTextAlignment alignment);