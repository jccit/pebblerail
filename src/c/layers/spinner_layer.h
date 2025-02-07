#pragma once

#include <pebble.h>

typedef Layer SpinnerLayer;

SpinnerLayer *spinner_layer_init(GRect bounds);
void spinner_layer_deinit(SpinnerLayer *spinner_layer);
