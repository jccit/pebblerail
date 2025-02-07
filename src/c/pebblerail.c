#include <pebble.h>
#include "windows/station_screen.h"
#include "windows/departures_screen.h"
#include "data.h"

void show_departures(char *crs)
{
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Showing departures for %s", crs);
  departures_screen_init(crs);
}

int main(void)
{
  data_init();
  station_screen_init(show_departures);

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing");

  app_event_loop();
}
