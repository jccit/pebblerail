#include <pebble.h>
#include "windows/station_screen.h"
#include "windows/departures_screen.h"
#include "data.h"

int main(void)
{
  data_init();
  station_screen_init();

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing");

  app_event_loop();
}
