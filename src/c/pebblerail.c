#include <pebble.h>
#include "station_screen.h"
#include "data.h"

void show_departures(char *crs)
{
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Showing departures for %s", crs);
}

int main(void)
{
  data_init();
  station_screen_init(show_departures);

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing");

  app_event_loop();
}
