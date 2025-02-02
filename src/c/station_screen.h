#pragma once

#include <pebble.h>

struct Station
{
  char name[30];
  char distance[9];
};

void station_screen_init(Window *window);
void station_screen_deinit();
