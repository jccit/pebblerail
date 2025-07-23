#pragma once

#include <pebble.h>

typedef struct StationScreen StationScreen;

StationScreen *station_screen_create();
void station_screen_destroy(StationScreen *layer);
void station_screen_push(StationScreen *screen);