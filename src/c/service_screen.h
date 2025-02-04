#pragma once

#include <pebble.h>

struct CallingPointEntry
{
  char destination[30];
  char departureTime[20];
  char platform[3];
};

void service_screen_init(char *serviceID);
void service_screen_deinit();
