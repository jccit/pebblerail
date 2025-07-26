#pragma once

#include <pebble.h>

#define LOG(fmt, args...) APP_LOG(APP_LOG_LEVEL_INFO, fmt, ##args);
#define LOG_WARN(fmt, args...) APP_LOG(APP_LOG_LEVEL_WARNING, fmt, ##args);
#define LOG_ERROR(fmt, args...) APP_LOG(APP_LOG_LEVEL_ERROR, fmt, ##args);

// aplite is memory constrained, avoid debug logging to reduce binary size
#ifdef PBL_PLATFORM_APLITE
#define LOG_DEBUG(fmt, args...)          // do nothing
#define LOG_DEBUG_VERBOSE(fmt, args...)  // do nothing
#else
#define LOG_DEBUG(fmt, args...) APP_LOG(APP_LOG_LEVEL_DEBUG, fmt, ##args);
#define LOG_DEBUG_VERBOSE(fmt, args...) APP_LOG(APP_LOG_LEVEL_DEBUG_VERBOSE, fmt, ##args);
#endif

GRect bounds_with_status_bar(Window *window);
GRect bounds_with_status_bar_no_padding(Window *window);
TextLayer *create_error_layer(Window *window, char *message);

GSize graphics_draw_text_get_size(GContext *ctx, char *text, GFont font, GRect bounds, GTextOverflowMode overflow_mode, GTextAlignment alignment);

void to_local_time(char *time, char *locale_time);

size_t check_free_memory();