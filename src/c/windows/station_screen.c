#include "station_screen.h"

#include "../data.h"
#include "../layers/menu_header.h"
#include "../layers/spinner_layer.h"
#include "../layers/status_bar.h"
#include "../utils.h"
#include "departures_screen.h"

#define ICON_TEST_ENABLED 0

static Window *s_window;
static StatusBarLayer *s_status_bar;
static Layer *s_spinner_layer;
static MenuLayer *s_menu_layer;

#if ICON_TEST_ENABLED
static Layer *s_icon_layer;
static GDrawCommandImage *s_large_icon;
static GDrawCommandImage *s_small_icon;
static GDrawCommandImage *s_tiny_icon;
#endif

#define STATION_COUNT 5
static struct Station s_stations[STATION_COUNT];
static uint8_t s_station_count = 0;

// ------ MENU LAYER CALLBACKS ------

static uint16_t menu_get_num_sections_callback(MenuLayer *menu_layer, void *data) { return 1; }

static uint16_t menu_get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) { return s_station_count; }

static void menu_draw_row_callback(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data) {
  menu_cell_basic_draw(ctx, cell_layer, s_stations[cell_index->row].name, s_stations[cell_index->row].distance, NULL);
}

static void menu_draw_header_callback(GContext *ctx, const Layer *cell_layer, uint16_t section_index, void *data) {
  menu_section_header_draw(ctx, cell_layer, "Closest stations");
}

static int16_t menu_get_header_height_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) { return 20; }

static void menu_select_click_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Showing departures for %s", s_stations[cell_index->row].crs);
  departures_screen_init(s_stations[cell_index->row].crs, s_stations[cell_index->row].name);
}

// ------ END MENU LAYER CALLBACKS ------

static void station_load_complete() {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Received all %d stations", STATION_COUNT);

  menu_layer_reload_data(s_menu_layer);
  layer_set_hidden(menu_layer_get_layer(s_menu_layer), false);
  layer_mark_dirty(menu_layer_get_layer(s_menu_layer));

  spinner_layer_deinit(s_spinner_layer);
}

static void closest_station_callback(DictionaryIterator *iter) {
  layer_set_hidden(menu_layer_get_layer(s_menu_layer), true);

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

  strncpy(s_stations[s_station_count].name, location_name, sizeof(s_stations[s_station_count].name) - 1);
  s_stations[s_station_count].name[sizeof(s_stations[s_station_count].name) - 1] = '\0';

  strncpy(s_stations[s_station_count].crs, crs, sizeof(s_stations[s_station_count].crs) - 1);
  s_stations[s_station_count].crs[sizeof(s_stations[s_station_count].crs) - 1] = '\0';

  strncpy(s_stations[s_station_count].distance, distance, sizeof(s_stations[s_station_count].distance) - 1);
  s_stations[s_station_count].distance[sizeof(s_stations[s_station_count].distance) - 1] = '\0';

  s_station_count++;

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Received station %d: %s, %s", s_station_count, s_stations[s_station_count - 1].name,
          s_stations[s_station_count - 1].distance);

  if (s_station_count == STATION_COUNT) {
    station_load_complete();
  }
}

void load_stations() {
  s_station_count = 0;

  set_closest_station_callback(closest_station_callback);
  request_closest_stations();
}

#if ICON_TEST_ENABLED
static void icon_layer_update_proc(Layer *layer, GContext *ctx) {
  gdraw_command_image_draw(ctx, s_tiny_icon, GPoint(30, 20));
  gdraw_command_image_draw(ctx, s_small_icon, GPoint(30, 40));
  gdraw_command_image_draw(ctx, s_large_icon, GPoint(30, 80));
}

void test_icons() {
  s_tiny_icon = gdraw_command_image_create_with_resource(RESOURCE_ID_TRAIN_TINY);
  s_small_icon = gdraw_command_image_create_with_resource(RESOURCE_ID_TRAIN_SMALL);
  s_large_icon = gdraw_command_image_create_with_resource(RESOURCE_ID_TRAIN_LARGE);

  s_icon_layer = layer_create(GRect(0, 0, PBL_DISPLAY_WIDTH, PBL_DISPLAY_HEIGHT));
  layer_set_update_proc(s_icon_layer, icon_layer_update_proc);
  layer_add_child(window_get_root_layer(s_window), s_icon_layer);
}
#endif
void station_window_load(Window *window) {
  s_status_bar = custom_status_bar_layer_create();

  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  GRect bounds_status_bar = bounds_with_status_bar(window);
  s_menu_layer = menu_layer_create(bounds_status_bar);

  menu_layer_set_callbacks(s_menu_layer, NULL,
                           (MenuLayerCallbacks){
                               .get_num_sections = menu_get_num_sections_callback,
                               .get_num_rows = menu_get_num_rows_callback,
                               .get_header_height = menu_get_header_height_callback,
                               .draw_row = menu_draw_row_callback,
                               .draw_header = menu_draw_header_callback,
                               .select_click = menu_select_click_callback,
                           });

  menu_layer_set_click_config_onto_window(s_menu_layer, window);
  layer_set_hidden(menu_layer_get_layer(s_menu_layer), true);

  s_spinner_layer = spinner_layer_init(bounds);

  layer_add_child(window_layer, menu_layer_get_layer(s_menu_layer));
  layer_add_child(window_layer, s_spinner_layer);
  layer_add_child(window_layer, status_bar_layer_get_layer(s_status_bar));

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Station screen initialized");

#if ICON_TEST_ENABLED
  test_icons();
#endif
}

void station_window_unload(Window *window) { station_screen_deinit(); }

void station_screen_init() {
  s_window = window_create();
  window_set_window_handlers(s_window, (WindowHandlers){
                                           .load = station_window_load,
                                           .unload = station_window_unload,
                                       });
  const bool animated = true;
  window_stack_push(s_window, animated);

  load_stations();
}

void station_screen_deinit() {
  custom_status_bar_layer_destroy(s_status_bar);
  menu_layer_destroy(s_menu_layer);
  window_destroy(s_window);
}
