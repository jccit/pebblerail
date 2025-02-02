#pragma once

#include <pebble.h>

struct Station
{
  char name[30];
  char crs[4];
  char distance[9];
};

void station_screen_init(void (*open_station_callback)(char *crs));
void station_screen_deinit();
