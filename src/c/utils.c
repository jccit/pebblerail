#include "utils.h"

GRect bounds_with_status_bar(Window *window)
{
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  return GRect(0, STATUS_BAR_LAYER_HEIGHT, bounds.size.w, bounds.size.h - STATUS_BAR_LAYER_HEIGHT);
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