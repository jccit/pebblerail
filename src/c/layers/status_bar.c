#include "status_bar.h"

StatusBarLayer *custom_status_bar_layer_create()
{
  StatusBarLayer *status_bar_layer = status_bar_layer_create();
  status_bar_layer_set_separator_mode(status_bar_layer, StatusBarLayerSeparatorModeDotted);
  status_bar_layer_set_colors(status_bar_layer, GColorWhite, GColorBlack);
  return status_bar_layer;
}

void custom_status_bar_set_color(StatusBarLayer *status_bar_layer, GColor bg_color)
{
  GColor fg_color = gcolor_legible_over(bg_color);
  status_bar_layer_set_colors(status_bar_layer, bg_color, fg_color);
}

void custom_status_bar_layer_destroy(StatusBarLayer *status_bar_layer)
{
  status_bar_layer_destroy(status_bar_layer);
}