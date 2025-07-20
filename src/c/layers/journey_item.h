#pragma once

#include <pebble.h>

#include "../data.h"

void journey_item_draw(GContext *ctx, const Layer *cell_layer, bool start, bool end, CallingPointEntry *callingPoint);