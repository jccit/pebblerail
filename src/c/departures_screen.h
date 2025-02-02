#pragma once

#include <pebble.h>

struct DepartureEntry
{
  char serviceID[10];
  char destination[30];
  char departureTime[6];
  char platform[3];
};

void departures_screen_init(char *crs);
void departures_screen_deinit();
