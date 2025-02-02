#pragma once

#include <pebble.h>

typedef enum
{
  COMMAND_TYPE_UNKNOWN = 0,
  COMMAND_TYPE_STATION_LIST = 1
} CommandType;

void data_init();

void set_closest_station_callback(void (*callback)(DictionaryIterator *iter));
void request_closest_stations();