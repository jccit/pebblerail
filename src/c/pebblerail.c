#include <pebble.h>

#include "data.h"
#include "windows/departures_screen.h"
#include "windows/station_screen.h"

int main(void) {
  data_init();
  StationScreen *station_screen = station_screen_create();
  station_screen_push(station_screen);

  app_event_loop();
}
