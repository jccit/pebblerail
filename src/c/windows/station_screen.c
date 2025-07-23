#include "station_screen.h"

#include "../data.h"
#include "../layers/menu_header.h"
#include "../layers/spinner_layer.h"
#include "../layers/status_bar.h"
#include "../utils.h"
#include "departures_screen.h"

#define MAX_STATION_COUNT 5

struct StationScreen {
  Window *window;
  StatusBarLayer *status_bar;
  Layer *spinner_layer;
  MenuLayer *menu_layer;
  Station *stations;
  uint8_t loaded_station_count;

  DeparturesScreen *departures_screen;
};

// ------ MENU LAYER CALLBACKS ------

static uint16_t menu_get_num_sections_callback(MenuLayer *menu_layer, void *data) { return 1; }

static uint16_t menu_get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) {
  StationScreen *screen = data;
  return screen->loaded_station_count;
}

static void menu_draw_row_callback(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data) {
  StationScreen *screen = data;
  menu_cell_basic_draw(ctx, cell_layer, screen->stations[cell_index->row].name, screen->stations[cell_index->row].distance, NULL);
}

static void menu_draw_header_callback(GContext *ctx, const Layer *cell_layer, uint16_t section_index, void *data) {
  menu_section_header_draw(ctx, cell_layer, "Closest stations");
}

static int16_t menu_get_header_height_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) { return 20; }

static void menu_select_click_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data) {
  StationScreen *screen = data;

  if (screen->loaded_station_count == 0) {
    return;
  }

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Showing departures for %s", screen->stations[cell_index->row].crs);

  screen->departures_screen = departures_screen_create(screen->stations[cell_index->row].crs, screen->stations[cell_index->row].name);
  departures_screen_push(screen->departures_screen);
}

// ------ END MENU LAYER CALLBACKS ------

static void station_load_complete(StationScreen *screen) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Received all %d stations", MAX_STATION_COUNT);

  menu_layer_reload_data(screen->menu_layer);
  layer_set_hidden(menu_layer_get_layer(screen->menu_layer), false);
  layer_mark_dirty(menu_layer_get_layer(screen->menu_layer));

  layer_set_hidden(screen->spinner_layer, true);
}

static void closest_station_callback(DictionaryIterator *iter, void *context) {
  StationScreen *screen = context;

  layer_set_hidden(menu_layer_get_layer(screen->menu_layer), true);

  Tuple *location_tuple = dict_find(iter, MESSAGE_KEY_locationName);
  if (!location_tuple) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "No station data received");
    return;
  }

  char *location_name = location_tuple->value->cstring;

  Tuple *crs_tuple = dict_find(iter, MESSAGE_KEY_crs);
  if (!crs_tuple) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "No crs data received");
    return;
  }

  char *crs = crs_tuple->value->cstring;

  Tuple *distance_tuple = dict_find(iter, MESSAGE_KEY_distance);
  if (!distance_tuple) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "No distance data received");
    return;
  }

  char *distance = distance_tuple->value->cstring;

  Station *stations = screen->stations;

  strncpy(stations[screen->loaded_station_count].name, location_name, sizeof(stations[screen->loaded_station_count].name) - 1);
  stations[screen->loaded_station_count].name[sizeof(stations[screen->loaded_station_count].name) - 1] = '\0';

  strncpy(stations[screen->loaded_station_count].crs, crs, sizeof(stations[screen->loaded_station_count].crs) - 1);
  stations[screen->loaded_station_count].crs[sizeof(stations[screen->loaded_station_count].crs) - 1] = '\0';

  strncpy(stations[screen->loaded_station_count].distance, distance, sizeof(stations[screen->loaded_station_count].distance) - 1);
  stations[screen->loaded_station_count].distance[sizeof(stations[screen->loaded_station_count].distance) - 1] = '\0';

  screen->loaded_station_count++;

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Received station %d: %s, %s", screen->loaded_station_count, stations[screen->loaded_station_count - 1].name,
          stations[screen->loaded_station_count - 1].distance);

  if (screen->loaded_station_count == MAX_STATION_COUNT) {
    station_load_complete(screen);
  }
}

void prv_load_stations(StationScreen *screen) {
  screen->loaded_station_count = 0;
  if (screen->stations != NULL) {
    free(screen->stations);
  }
  screen->stations = malloc(sizeof(Station) * MAX_STATION_COUNT);

  set_closest_station_callback(closest_station_callback, (void *)screen);
  request_closest_stations();
}

void station_window_appear(Window *window) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Station screen appear");
  StationScreen *screen = window_get_user_data(window);

  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  GRect bounds_status_bar = bounds_with_status_bar(window);

  screen->status_bar = custom_status_bar_layer_create();
  screen->menu_layer = menu_layer_create(bounds_status_bar);
  screen->spinner_layer = spinner_layer_init(bounds);

  menu_layer_set_callbacks(screen->menu_layer, screen,
                           (MenuLayerCallbacks){
                               .get_num_sections = menu_get_num_sections_callback,
                               .get_num_rows = menu_get_num_rows_callback,
                               .get_header_height = menu_get_header_height_callback,
                               .draw_row = menu_draw_row_callback,
                               .draw_header = menu_draw_header_callback,
                               .select_click = menu_select_click_callback,
                           });

  menu_layer_set_click_config_onto_window(screen->menu_layer, window);
  layer_set_hidden(menu_layer_get_layer(screen->menu_layer), true);

  layer_add_child(window_layer, menu_layer_get_layer(screen->menu_layer));
  layer_add_child(window_layer, screen->spinner_layer);
  layer_add_child(window_layer, status_bar_layer_get_layer(screen->status_bar));

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Station screen appear complete");

  if (screen->loaded_station_count > 0) {
    station_load_complete(screen);
  }
}

void station_window_disappear(Window *window) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Station screen disappear");
  StationScreen *screen = window_get_user_data(window);

  custom_status_bar_layer_destroy(screen->status_bar);
  menu_layer_destroy(screen->menu_layer);
  spinner_layer_deinit(screen->spinner_layer);

  screen->spinner_layer = NULL;
  screen->status_bar = NULL;
  screen->menu_layer = NULL;
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Station screen disappear complete");
}

void station_screen_destroy(StationScreen *screen) {
  window_destroy(screen->window);
  free(screen->stations);
  free(screen);
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Station screen destroyed");
}

void station_window_unload(Window *window) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Station screen unload");
  StationScreen *screen = window_get_user_data(window);
  station_screen_destroy(screen);
}

StationScreen *station_screen_create() {
  StationScreen *screen = malloc(sizeof(StationScreen));
  screen->window = window_create();

  window_set_window_handlers(screen->window, (WindowHandlers){
                                                 .unload = station_window_unload,
                                                 .appear = station_window_appear,
                                                 .disappear = station_window_disappear,
                                             });

  window_set_user_data(screen->window, screen);

  prv_load_stations(screen);

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Station screen created");

  return screen;
}

void station_screen_push(StationScreen *screen) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Station screen pushed");
  window_stack_push(screen->window, true);
}