#pragma once

#include <pebble.h>

#include "../data.h"

#ifdef PBL_ROUND
Layer *round_route_path_init(GRect bounds);
void round_route_path_set_data(Layer *layer, CallingPointEntry *callingPoints, int callingPointCount, char *operatorCode);
bool round_route_next_calling_point(Layer *layer);
bool round_route_previous_calling_point(Layer *layer);
#endif