#pragma once

#include <pebble.h>

#include "data.h"

SavedStation *persist_get_stations();
uint32_t persist_get_station_count();

void persist_save_station(Station *station);
void persist_delete_station(char *crs);