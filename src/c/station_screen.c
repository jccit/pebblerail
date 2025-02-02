#include "station_screen.h"
#include "data.h"

static MenuLayer *s_menu_layer;

#define STATION_COUNT 5
static struct Station s_stations[STATION_COUNT];
static uint8_t s_station_count = 0;

// ------ MENU LAYER CALLBACKS ------

static uint16_t menu_get_num_sections_callback(MenuLayer *menu_layer, void *data)
{
  return 1;
}

static uint16_t menu_get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *data)
{
  return s_station_count;
}

static void menu_draw_row_callback(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data)
{
  menu_cell_basic_draw(ctx, cell_layer, s_stations[cell_index->row].name, s_stations[cell_index->row].distance, NULL);
}

static void menu_draw_header_callback(GContext *ctx, const Layer *cell_layer, uint16_t section_index, void *data)
{
  menu_cell_basic_header_draw(ctx, cell_layer, "Closest stations");
}

// ------ END MENU LAYER CALLBACKS ------

static void closest_station_callback(DictionaryIterator *iter)
{
  Tuple *data_t = dict_find(iter, MESSAGE_KEY_stationList);
  if (!data_t)
  {
    APP_LOG(APP_LOG_LEVEL_ERROR, "No station data received");
    return;
  }

  char *station_str = data_t->value->cstring;
  char *separator = strchr(station_str, ';');
  if (separator == NULL)
  {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Invalid station data format: %s", station_str);
    return;
  }

  // Extract and store the station name.
  size_t name_length = separator - station_str;
  if (name_length >= sizeof(s_stations[s_station_count].name))
  {
    name_length = sizeof(s_stations[s_station_count].name) - 1;
  }
  memcpy(s_stations[s_station_count].name, station_str, name_length);
  s_stations[s_station_count].name[name_length] = '\0';

  // Extract and store the distance.
  char *distance_str = separator + 1;
  strncpy(s_stations[s_station_count].distance, distance_str, sizeof(s_stations[s_station_count].distance) - 1);
  s_stations[s_station_count].distance[sizeof(s_stations[s_station_count].distance) - 1] = '\0';

  s_station_count++;

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Received station %d: %s, %s", s_station_count, s_stations[s_station_count - 1].name, s_stations[s_station_count - 1].distance);

  if (s_station_count == STATION_COUNT)
  {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Received all %d stations", STATION_COUNT);
    menu_layer_reload_data(s_menu_layer);
    layer_mark_dirty(menu_layer_get_layer(s_menu_layer));
  }
}

void load_stations()
{
  s_station_count = 0;

  set_closest_station_callback(closest_station_callback);
  request_closest_stations();
}

void station_screen_init(Window *window)
{
  load_stations();

  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  s_menu_layer = menu_layer_create(GRect(0, STATUS_BAR_LAYER_HEIGHT, bounds.size.w, bounds.size.h - STATUS_BAR_LAYER_HEIGHT));

  menu_layer_set_callbacks(s_menu_layer, NULL, (MenuLayerCallbacks){
                                                   .get_num_sections = menu_get_num_sections_callback,
                                                   .get_num_rows = menu_get_num_rows_callback,
                                                   .draw_row = menu_draw_row_callback,
                                                   .draw_header = menu_draw_header_callback,
                                               });

  menu_layer_set_click_config_onto_window(s_menu_layer, window);

  layer_add_child(window_layer, menu_layer_get_layer(s_menu_layer));

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Station screen initialized");
}

void station_screen_deinit()
{
  menu_layer_destroy(s_menu_layer);
}
