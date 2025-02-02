#include <pebble.h>
#include "station_screen.h"
#include "data.h"

static Window *s_window;

static void prv_window_load(Window *window)
{
  station_screen_init(window);
}

static void prv_window_unload(Window *window)
{
  station_screen_deinit();
}

static void prv_init(void)
{
  s_window = window_create();
  window_set_window_handlers(s_window, (WindowHandlers){
                                           .load = prv_window_load,
                                           .unload = prv_window_unload,
                                       });
  const bool animated = true;
  window_stack_push(s_window, animated);
}

static void prv_deinit(void)
{
  window_destroy(s_window);
}

int main(void)
{
  data_init();
  prv_init();

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", s_window);

  app_event_loop();
  prv_deinit();
}
