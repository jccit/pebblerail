#include "utils.h"

#include <string.h>

GRect _status_bar_bounds(Window *window, int status_bar_height) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  return GRect(0, status_bar_height, bounds.size.w, bounds.size.h - status_bar_height);
}

GRect bounds_with_status_bar(Window *window) { return _status_bar_bounds(window, STATUS_BAR_LAYER_HEIGHT); }

GRect bounds_with_status_bar_no_padding(Window *window) { return _status_bar_bounds(window, STATUS_BAR_LAYER_HEIGHT / 2); }

TextLayer *create_error_layer(Window *window, char *message) {
  GRect status_bar_bounds = bounds_with_status_bar(window);
  GRect bounds = GRect(status_bar_bounds.origin.x, status_bar_bounds.size.h / 2, status_bar_bounds.size.w, 14);
  TextLayer *error_layer = text_layer_create(bounds);

  text_layer_set_text(error_layer, message);
  text_layer_set_text_alignment(error_layer, GTextAlignmentCenter);
  return error_layer;
}

/**
 * Draws text and returns the size of the text
 */
GSize graphics_draw_text_get_size(GContext *ctx, char *text, GFont font, GRect bounds, GTextOverflowMode overflow_mode, GTextAlignment alignment) {
  GSize size = graphics_text_layout_get_content_size(text, font, bounds, overflow_mode, alignment);
  graphics_draw_text(ctx, text, font, bounds, overflow_mode, alignment, NULL);
  return size;
}

void to_local_time(char *time_str, char *locale_time) {
  // time will be a 24 hour utc string like '17:30'
  // there could be extra stuff after the time like '17:30 +1'
  if (strlen(time_str) < 4) {
    strcpy(locale_time, time_str);
    return;
  }

  char *minutes_ptr = strstr(time_str, ":");

  char hours[3];
  char minutes[3];

  strncpy(hours, time_str, 2);
  strncpy(minutes, minutes_ptr + 1, 2);

  // ensure the strings are null terminated
  hours[2] = '\0';
  minutes[2] = '\0';

  // convert the hours and minutes to integers
  int hours_int = atoi(hours);
  int minutes_int = atoi(minutes);

  if (hours_int > 23 || hours_int < 0) {
    hours_int = 0;
  }

  if (minutes_int > 59 || minutes_int < 0) {
    minutes_int = 0;
  }

  // construct a tm object in utc
  time_t now = time(NULL);
  struct tm *utc_tm = gmtime(&now);
  utc_tm->tm_hour = hours_int;
  utc_tm->tm_min = minutes_int;

  // convert utc to local time
  time_t utc_time = mktime(utc_tm);
  struct tm *local_tm = localtime(&utc_time);

  strftime(locale_time, 6, "%H:%M", local_tm);
}