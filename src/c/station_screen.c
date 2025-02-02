#include "station_screen.h"
#include "data.h"

static MenuLayer *s_menu_layer;

// ------ MENU LAYER CALLBACKS ------

static uint16_t menu_get_num_sections_callback(MenuLayer *menu_layer, void *data)
{
  return 1;
}

static uint16_t menu_get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *data)
{
  return 3;
}

static void menu_draw_row_callback(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data)
{
  // TODO: Draw the row
  menu_cell_basic_draw(ctx, cell_layer, "Station", "123456", NULL);
}

static void menu_draw_header_callback(GContext *ctx, const Layer *cell_layer, uint16_t section_index, void *data)
{
  // TODO: Draw the header
}

// ------ END MENU LAYER CALLBACKS ------

static void closest_station_callback(DictionaryIterator *iter)
{
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Got closest station callback");
}

void station_screen_init(Window *window)
{
  set_closest_station_callback(closest_station_callback);
  request_closest_stations();

  Layer *window_layer = window_get_root_layer(window);
  s_menu_layer = menu_layer_create(GRect(0, 0, 144, 168)); // TODO: Make this dynamic to fit round

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
