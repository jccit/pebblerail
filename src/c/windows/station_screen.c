#include "station_screen.h"

#include "../data.h"
#include "../layers/menu_header.h"
#include "../layers/spinner_layer.h"
#include "../layers/status_bar.h"
#include "../persist.h"
#include "../utils.h"
#include "../window_manager.h"
#include "departures_screen.h"

#define MAX_STATION_COUNT 5
#define MAX_SAVED_STATION_COUNT 5
#define STATION_ACTION_MENU_NUM_ITEMS 2

typedef enum { MENU_ACTION_VIEW_DEPARTURES = 1, MENU_ACTION_SAVE = 2, MENU_ACTION_DELETE = 3 } StationMenuAction;

struct StationScreen {
  Window *window;
  StatusBarLayer *status_bar;
  Layer *spinner_layer;
  MenuLayer *menu_layer;

  Station *stations;
  SavedStation *saved_stations;
  bool load_complete;
  uint8_t loaded_station_count;
  uint8_t saved_station_count;

  ActionMenu *action_menu;
  ActionMenuLevel *root_level;
  uint8_t selected_station_index;
};
bool has_saved_stations(StationScreen *screen) { return screen->saved_station_count > 0; }

bool is_saved_station(StationScreen *screen, char *crs) {
  if (!has_saved_stations(screen)) return false;

  for (int i = 0; i < screen->saved_station_count; i++) {
    if (strcmp(screen->saved_stations[i].crs, crs) == 0) {
      return true;
    }
  }
  return false;
}

void prv_load_saved_stations(StationScreen *screen) {
  screen->saved_station_count = 0;

  if (screen->saved_stations != NULL) {
    free(screen->saved_stations);
    screen->saved_stations = NULL;
  }

  screen->saved_stations = persist_get_stations();
  screen->saved_station_count = persist_get_station_count();

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Loaded %d saved stations", screen->saved_station_count);

  if (screen->saved_station_count > 0) {
    if (screen->menu_layer != NULL) {
      menu_layer_reload_data(screen->menu_layer);
    }
    if (screen->spinner_layer != NULL) {
      layer_set_hidden(screen->spinner_layer, true);
    }
  }
}

static void action_menu_close_callback(ActionMenu *menu, const ActionMenuItem *performed_action, void *context) {
  StationScreen *screen = context;
  screen->action_menu = NULL;
}

static void station_action_performed_callback(ActionMenu *action_menu, const ActionMenuItem *action, void *context) {
  StationScreen *screen = context;
  StationMenuAction selected_action = (StationMenuAction)action_menu_item_get_action_data(action);
  LOG_DEBUG("Selected action: %d", selected_action);

  Station *selected_station = &screen->stations[screen->selected_station_index];

  if (selected_action == MENU_ACTION_VIEW_DEPARTURES) {
    DeparturesScreen *departures_screen = departures_screen_create(selected_station->crs, selected_station->name);
    departures_screen_push(departures_screen);
  } else if (selected_action == MENU_ACTION_SAVE) {
    persist_save_station(selected_station);
    prv_load_saved_stations(screen);
  } else if (selected_action == MENU_ACTION_DELETE) {
    persist_delete_station(selected_station->crs);
    prv_load_saved_stations(screen);
  }
}

// ------ MENU LAYER CALLBACKS ------

static uint16_t menu_get_num_sections_callback(MenuLayer *menu_layer, void *data) {
  StationScreen *screen = data;

  return has_saved_stations(screen) ? 2 : 1;
}

static uint16_t menu_get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) {
  StationScreen *screen = data;

  if (has_saved_stations(screen)) {
    if (section_index == 0) {
      return screen->saved_station_count;
    } else {
      return screen->load_complete ? screen->loaded_station_count : 1;
    }
  }

  return screen->load_complete ? screen->loaded_station_count : 0;
}

static void menu_draw_row_callback(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data) {
  StationScreen *screen = data;

  if (!has_saved_stations(screen)) {
    menu_cell_basic_draw(ctx, cell_layer, screen->stations[cell_index->row].name, screen->stations[cell_index->row].distance, NULL);
    return;
  }

  if (cell_index->section == 0) {
    menu_cell_basic_draw(ctx, cell_layer, screen->saved_stations[cell_index->row].name, NULL, NULL);
  } else {
    if (screen->load_complete) {
      menu_cell_basic_draw(ctx, cell_layer, screen->stations[cell_index->row].name, screen->stations[cell_index->row].distance, NULL);
    } else {
      menu_cell_basic_draw(ctx, cell_layer, "Loading...", NULL, NULL);
    }
  }
}

static void menu_draw_header_callback(GContext *ctx, const Layer *cell_layer, uint16_t section_index, void *data) {
  StationScreen *screen = data;

  if (!has_saved_stations(screen)) {
    menu_section_header_draw(ctx, cell_layer, "Closest stations");
    return;
  }

  if (section_index == 0) {
    menu_section_header_draw(ctx, cell_layer, "Favourites");
  } else {
    menu_section_header_draw(ctx, cell_layer, "Closest stations");
  }
}

static int16_t menu_get_header_height_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) { return 20; }

static void menu_select_click_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data) {
  StationScreen *screen = data;
  bool is_saved_section = has_saved_stations(screen) && cell_index->section == 0;
  bool is_saved = is_saved_station(screen, screen->stations[cell_index->row].crs);

  if (screen->loaded_station_count == 0 && !is_saved_section) {
    return;
  }

  if (screen->root_level != NULL) {
    free(screen->root_level);
    screen->root_level = NULL;
  }

  screen->root_level = action_menu_level_create(STATION_ACTION_MENU_NUM_ITEMS);

  action_menu_level_add_action(screen->root_level, "View stops", station_action_performed_callback, (void *)MENU_ACTION_VIEW_DEPARTURES);

  if (is_saved_section || is_saved) {
    action_menu_level_add_action(screen->root_level, "Unfavourite", station_action_performed_callback, (void *)MENU_ACTION_DELETE);
  } else {
    action_menu_level_add_action(screen->root_level, "Favourite", station_action_performed_callback, (void *)MENU_ACTION_SAVE);
  }

  ActionMenuConfig config = (ActionMenuConfig){.root_level = screen->root_level,
                                               .colors =
                                                   {
                                                       .background = GColorWhite,
                                                       .foreground = GColorBlack,
                                                   },
                                               .align = ActionMenuAlignCenter,
                                               .did_close = action_menu_close_callback,
                                               .context = screen};

  // Show the ActionMenu
  screen->selected_station_index = cell_index->row;
  screen->action_menu = action_menu_open(&config);
}

// ------ END MENU LAYER CALLBACKS ------

static void station_load_complete(StationScreen *screen) {
  LOG("Received all %d stations", MAX_STATION_COUNT);
  screen->load_complete = true;

  menu_layer_reload_data(screen->menu_layer);
  layer_set_hidden(menu_layer_get_layer(screen->menu_layer), false);
  layer_mark_dirty(menu_layer_get_layer(screen->menu_layer));

  layer_set_hidden(screen->spinner_layer, true);
}

static void closest_station_callback(DictionaryIterator *iter, void *context) {
  StationScreen *screen = context;

  Tuple *location_tuple = dict_find(iter, MESSAGE_KEY_locationName);
  if (!location_tuple) {
    LOG_ERROR("No station data received");
    return;
  }

  char *location_name = location_tuple->value->cstring;

  Tuple *crs_tuple = dict_find(iter, MESSAGE_KEY_crs);
  if (!crs_tuple) {
    LOG_ERROR("No crs data received");
    return;
  }

  char *crs = crs_tuple->value->cstring;

  Tuple *distance_tuple = dict_find(iter, MESSAGE_KEY_distance);
  if (!distance_tuple) {
    LOG_ERROR("No distance data received");
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

  LOG_DEBUG("Received station %d: %s, %s", screen->loaded_station_count, stations[screen->loaded_station_count - 1].name,
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
  screen->stations = wm_alloc(sizeof(Station) * MAX_STATION_COUNT);

  set_closest_station_callback(closest_station_callback, (void *)screen);
  request_closest_stations();
}

void station_window_appear(Window *window) {
  LOG_DEBUG("Station screen appear");
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

  layer_add_child(window_layer, menu_layer_get_layer(screen->menu_layer));
  layer_add_child(window_layer, screen->spinner_layer);
  layer_add_child(window_layer, status_bar_layer_get_layer(screen->status_bar));

  LOG_DEBUG("Station screen appear complete");

  prv_load_saved_stations(screen);

  if (screen->loaded_station_count > 0) {
    station_load_complete(screen);
  }
}

void station_window_disappear(Window *window) {
  LOG_DEBUG("Station screen disappear");
  StationScreen *screen = window_get_user_data(window);

  custom_status_bar_layer_destroy(screen->status_bar);
  menu_layer_destroy(screen->menu_layer);
  spinner_layer_deinit(screen->spinner_layer);

  screen->spinner_layer = NULL;
  screen->status_bar = NULL;
  screen->menu_layer = NULL;
  LOG_DEBUG("Station screen disappear complete");
}

void station_screen_destroy(StationScreen *screen) {
  if (screen->action_menu != NULL) {
    free(screen->action_menu);
    screen->action_menu = NULL;
  }

  if (screen->root_level != NULL) {
    free(screen->root_level);
    screen->root_level = NULL;
  }

  window_destroy(screen->window);
  free(screen->saved_stations);
  free(screen->stations);
  free(screen);
  LOG_DEBUG("Station screen destroyed");
}

void station_window_unload(Window *window) {
  LOG_DEBUG("Station screen unload");
  StationScreen *screen = window_get_user_data(window);
  station_screen_destroy(screen);
}

StationScreen *station_screen_create() {
  StationScreen *screen = wm_alloc(sizeof(StationScreen));
  screen->window = window_create();
  screen->load_complete = false;

  window_set_window_handlers(screen->window, (WindowHandlers){
                                                 .unload = station_window_unload,
                                                 .appear = station_window_appear,
                                                 .disappear = station_window_disappear,
                                             });

  window_set_user_data(screen->window, screen);

  prv_load_stations(screen);

  LOG_DEBUG("Station screen created");

  return screen;
}

void station_screen_push(StationScreen *screen) {
  LOG_DEBUG("Station screen pushed");
  window_manager_push_window(screen->window);
}