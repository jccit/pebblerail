#pragma once

#include <pebble.h>

#define EXTRACT_TUPLE(iter, key, var)                          \
  Tuple *var##_tuple = dict_find(iter, MESSAGE_KEY_##key);     \
  if (!var##_tuple)                                            \
  {                                                            \
    APP_LOG(APP_LOG_LEVEL_ERROR, "No " #key " data received"); \
    return;                                                    \
  }                                                            \
  char *var = var##_tuple->value->cstring;

#define COPY_STRING(dest, src)          \
  strncpy(dest, src, sizeof(dest) - 1); \
  dest[sizeof(dest) - 1] = '\0';

typedef enum
{
  COMMAND_TYPE_UNKNOWN = 0,
  COMMAND_TYPE_STATION_LIST = 1,
  COMMAND_TYPE_DEPARTURES = 2,
  COMMAND_TYPE_SERVICE = 3,
  COMMAND_TYPE_PIN_CALLING_POINT = 4
} CommandType;

typedef struct CallingPointEntry
{
  char destination[30];
  char departureTime[20];
  char platform[3];
  char crs[4];
  bool skipped;
} CallingPointEntry;

typedef struct ServiceInfo
{
  char serviceID[20];
  char origin[4];
  char destination[4];
  char operatorCode[3];
  char cancelReason[64];
  char delayReason[64];
  bool isCancelled;
} ServiceInfo;

typedef struct Station
{
  char name[30];
  char crs[4];
  char distance[9];
} Station;

typedef struct DepartureEntry
{
  char serviceID[20];
  char destination[30];
  char departureTime[6];
  char platform[3];
  char operatorCode[3];
} DepartureEntry;

void data_init();

void set_closest_station_callback(void (*callback)(DictionaryIterator *iter));
void request_closest_stations();

void set_departures_callback(void (*callback)(DictionaryIterator *iter));
void request_departures(char *crs);

void set_service_info_callback(void (*callback)(DictionaryIterator *iter));
void set_calling_point_callback(void (*callback)(DictionaryIterator *iter));
void request_service(char *service_id);

void pin_calling_point(char *service_id, char *crs, bool isArrival);
