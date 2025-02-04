#pragma once

#include <pebble.h>

typedef enum
{
  COMMAND_TYPE_UNKNOWN = 0,
  COMMAND_TYPE_STATION_LIST = 1,
  COMMAND_TYPE_DEPARTURES = 2,
  COMMAND_TYPE_SERVICE = 3,
  COMMAND_TYPE_PIN_CALLING_POINT = 4
} CommandType;

void data_init();

void set_closest_station_callback(void (*callback)(DictionaryIterator *iter));
void request_closest_stations();

void set_departures_callback(void (*callback)(DictionaryIterator *iter));
void request_departures(char *crs);

void set_service_callback(void (*callback)(DictionaryIterator *iter));
void request_service(char *service_id);

void pin_calling_point(char *service_id, char *crs, bool isArrival);
