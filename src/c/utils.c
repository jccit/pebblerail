#include "utils.h"

GRect _status_bar_bounds(Window *window, int status_bar_height)
{
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  return GRect(0, status_bar_height, bounds.size.w, bounds.size.h - status_bar_height);
}

GRect bounds_with_status_bar(Window *window)
{
  return _status_bar_bounds(window, STATUS_BAR_LAYER_HEIGHT);
}

GRect bounds_with_status_bar_no_padding(Window *window)
{
  return _status_bar_bounds(window, STATUS_BAR_LAYER_HEIGHT / 2);
}

TextLayer *create_error_layer(Window *window, char *message)
{
  GRect status_bar_bounds = bounds_with_status_bar(window);
  GRect bounds = GRect(status_bar_bounds.origin.x, status_bar_bounds.size.h / 2, status_bar_bounds.size.w, 14);
  TextLayer *error_layer = text_layer_create(bounds);

  text_layer_set_text(error_layer, message);
  text_layer_set_text_alignment(error_layer, GTextAlignmentCenter);
  return error_layer;
}