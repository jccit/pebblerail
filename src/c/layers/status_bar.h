#pragma once

#include <pebble.h>

StatusBarLayer *custom_status_bar_layer_create();
void custom_status_bar_set_color(StatusBarLayer *status_bar_layer, GColor bg_color);
void custom_status_bar_layer_destroy(StatusBarLayer *status_bar_layer);