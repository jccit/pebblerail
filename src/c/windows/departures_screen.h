#pragma once

#include <pebble.h>

typedef struct DeparturesScreen DeparturesScreen;

DeparturesScreen *departures_screen_create(char *crs, char *stationName);
void departures_screen_destroy(DeparturesScreen *screen);
void departures_screen_push(DeparturesScreen *screen);
