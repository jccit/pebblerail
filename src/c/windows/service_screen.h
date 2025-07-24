#pragma once

#include <pebble.h>

typedef struct ServiceScreen ServiceScreen;

ServiceScreen *service_screen_create(char *serviceID, char *origin);
void service_screen_destroy(ServiceScreen *screen);
void service_screen_push(ServiceScreen *screen);

ServiceScreen *top_window_as_service_screen();