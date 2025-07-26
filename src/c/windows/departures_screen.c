#include "departures_screen.h"

#include "../data.h"
#include "../layers/menu_header.h"
#include "../layers/spinner_layer.h"
#include "../layers/status_bar.h"
#include "../utils.h"
#include "../window_manager.h"
#include "service_screen.h"

#define ACTION_MENU_NUM_ITEMS 2
#define MAX_DEPARTURE_COUNT 10

typedef enum { MENU_ACTION_VIEW_STOPS = 1, MENU_ACTION_PIN = 2 } DepartureMenuAction;

struct DeparturesScreen {
  Window *window;
  StatusBarLayer *status_bar;
  MenuLayer *menu_layer;
  Layer *spinner_layer;
  TextLayer *error_layer;

  char *crs;
  char *station_name;

  ActionMenu *action_menu;
  ActionMenuLevel *root_level;
  uint8_t selected_departure_index;

  struct DepartureEntry departures[MAX_DEPARTURE_COUNT];
  uint8_t departure_count;
  uint8_t available_departures;
  bool load_complete;
};

static void action_performed_callback(ActionMenu *action_menu, const ActionMenuItem *action, void *context) {
  DeparturesScreen *screen = context;
  DepartureMenuAction selected_action = (DepartureMenuAction)action_menu_item_get_action_data(action);
  LOG_DEBUG("Selected action: %d", selected_action);

  if (selected_action == MENU_ACTION_VIEW_STOPS) {
    ServiceScreen *service_screen = service_screen_create(screen->departures[screen->selected_departure_index].serviceID, screen->crs);
    service_screen_push(service_screen);
  } else if (selected_action == MENU_ACTION_PIN) {
    pin_calling_point(screen->departures[screen->selected_departure_index].serviceID, screen->crs, false);
  }
}

// ------ MENU LAYER CALLBACKS ------

static uint16_t menu_get_num_sections_callback(MenuLayer *menu_layer, void *context) { return 1; }

static uint16_t menu_get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *context) {
  DeparturesScreen *screen = context;
  return screen->departure_count;
}

static void menu_draw_row_callback(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *context) {
  DeparturesScreen *screen = context;

  char combined_text[32];
  int index = cell_index->row;
  char *platform_display = screen->departures[index].platform;

  if (strcmp(platform_display, "un") == 0) {
    platform_display = "?";
  }

  snprintf(combined_text, sizeof(combined_text), "Dep %s - Pl.%s", screen->departures[index].departureTime, platform_display);

  menu_cell_basic_draw(ctx, cell_layer, screen->departures[index].destination, combined_text, NULL);
}

static void menu_draw_header_callback(GContext *ctx, const Layer *cell_layer, uint16_t section_index, void *context) {
  DeparturesScreen *screen = context;
  menu_section_header_draw(ctx, cell_layer, screen->station_name);
}

static int16_t menu_get_header_height_callback(MenuLayer *menu_layer, uint16_t section_index, void *context) { return 20; }

static void action_menu_close_callback(ActionMenu *menu, const ActionMenuItem *performed_action, void *context) {
  DeparturesScreen *screen = context;
  screen->action_menu = NULL;
}

static void menu_select_click_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *context) {
  DeparturesScreen *screen = context;

  // If the menu is hidden, don't allow clicks
  if (layer_get_hidden(menu_layer_get_layer(screen->menu_layer))) {
    return;
  }

  if (screen->root_level == NULL) {
    screen->root_level = action_menu_level_create(ACTION_MENU_NUM_ITEMS);

    action_menu_level_add_action(screen->root_level, "View stops", action_performed_callback, (void *)MENU_ACTION_VIEW_STOPS);
    action_menu_level_add_action(screen->root_level, "Pin to timeline", action_performed_callback, (void *)MENU_ACTION_PIN);
  }

  // Configure the ActionMenu Window about to be shown
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
  screen->selected_departure_index = cell_index->row;
  screen->action_menu = action_menu_open(&config);
}

// ------ END MENU LAYER CALLBACKS ------

static void departures_load_complete(DeparturesScreen *screen) {
  LOG_DEBUG("Received all %d departures", screen->available_departures);
  screen->load_complete = true;

  menu_layer_reload_data(screen->menu_layer);
  layer_set_hidden(menu_layer_get_layer(screen->menu_layer), false);
  layer_mark_dirty(menu_layer_get_layer(screen->menu_layer));

  layer_set_hidden(screen->spinner_layer, true);
}

static void no_departures(DeparturesScreen *screen) {
  departures_load_complete(screen);
  layer_set_hidden(menu_layer_get_layer(screen->menu_layer), true);

  screen->error_layer = create_error_layer(screen->window, "No departures found");
  layer_add_child(window_get_root_layer(screen->window), text_layer_get_layer(screen->error_layer));
}

static void departures_callback(DictionaryIterator *iter, void *context) {
  DeparturesScreen *screen = context;

  layer_set_hidden(menu_layer_get_layer(screen->menu_layer), true);

  Tuple *count_tuple = dict_find(iter, MESSAGE_KEY_count);
  if (!count_tuple) {
    LOG_ERROR("No departure data received");
    return;
  }
  uint8_t count = count_tuple->value->uint8;

  screen->available_departures = count > MAX_DEPARTURE_COUNT ? MAX_DEPARTURE_COUNT : count;

  LOG("Available departures = %d", screen->available_departures);

  if (screen->available_departures == 0) {
    LOG_WARN("No departures");
    no_departures(screen);
    return;
  }

  EXTRACT_TUPLE(iter, serviceID, serviceID);
  EXTRACT_TUPLE(iter, locationName, locationName);
  EXTRACT_TUPLE(iter, time, time);
  EXTRACT_TUPLE(iter, platform, platform);
  EXTRACT_TUPLE(iter, operatorCode, operatorCode);

  COPY_STRING(screen->departures[screen->departure_count].destination, locationName);
  COPY_STRING(screen->departures[screen->departure_count].departureTime, time);
  COPY_STRING(screen->departures[screen->departure_count].platform, platform);
  COPY_STRING(screen->departures[screen->departure_count].serviceID, serviceID);
  COPY_STRING(screen->departures[screen->departure_count].operatorCode, operatorCode);

  to_local_time(time, screen->departures[screen->departure_count].departureTime);

  screen->departure_count++;

  LOG_DEBUG_VERBOSE("Received departure %d: %s, %s, %s, %s", screen->departure_count, screen->departures[screen->departure_count - 1].serviceID,
                    screen->departures[screen->departure_count - 1].destination, screen->departures[screen->departure_count - 1].departureTime,
                    screen->departures[screen->departure_count - 1].platform);

  if (screen->departure_count == screen->available_departures) {
    departures_load_complete(screen);
  }
}

void prv_load_departures(DeparturesScreen *screen) {
  screen->departure_count = 0;

  set_departures_callback(departures_callback, (void *)screen);
  request_departures(screen->crs);
}

void departures_window_appear(Window *window) {
  DeparturesScreen *screen = window_get_user_data(window);

  LOG_DEBUG("Departures window appear");

  screen->status_bar = custom_status_bar_layer_create();

  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  GRect bounds_status_bar = bounds_with_status_bar(window);
  screen->menu_layer = menu_layer_create(bounds_status_bar);

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

  screen->spinner_layer = spinner_layer_init(bounds);

  layer_add_child(window_layer, menu_layer_get_layer(screen->menu_layer));
  layer_add_child(window_layer, screen->spinner_layer);
  layer_add_child(window_layer, status_bar_layer_get_layer(screen->status_bar));

  if (screen->load_complete) {
    departures_load_complete(screen);
  }

  LOG_DEBUG("Departures window appear complete");
}

void departures_window_disappear(Window *window) {
  DeparturesScreen *screen = window_get_user_data(window);

  if (screen->status_bar != NULL) {
    custom_status_bar_layer_destroy(screen->status_bar);
    screen->status_bar = NULL;
  }

  if (screen->menu_layer != NULL) {
    menu_layer_destroy(screen->menu_layer);
    screen->menu_layer = NULL;
  }

  if (screen->spinner_layer != NULL) {
    spinner_layer_deinit(screen->spinner_layer);
    screen->spinner_layer = NULL;
  }

  if (screen->error_layer != NULL) {
    text_layer_destroy(screen->error_layer);
    screen->error_layer = NULL;
  }
}

void departures_screen_destroy(DeparturesScreen *screen) {
  if (screen->action_menu != NULL) {
    free(screen->action_menu);
    screen->action_menu = NULL;
  }

  if (screen->root_level != NULL) {
    free(screen->root_level);
    screen->root_level = NULL;
  }

  window_destroy(screen->window);
  free(screen);
  LOG_DEBUG("Departures screen destroyed");
}

void departures_window_unload(Window *window) {
  LOG_DEBUG("Departures screen unload");
  DeparturesScreen *screen = window_get_user_data(window);
  departures_screen_destroy(screen);
}

DeparturesScreen *departures_screen_create(char *crs, char *station_name) {
  DeparturesScreen *screen = wm_alloc(sizeof(DeparturesScreen));

  screen->crs = crs;
  screen->station_name = station_name;

  screen->status_bar = NULL;
  screen->menu_layer = NULL;
  screen->spinner_layer = NULL;
  screen->error_layer = NULL;
  screen->action_menu = NULL;
  screen->root_level = NULL;
  screen->load_complete = false;

  screen->window = window_create();

  window_set_window_handlers(screen->window, (WindowHandlers){
                                                 .appear = departures_window_appear,
                                                 .disappear = departures_window_disappear,
                                                 .unload = departures_window_unload,
                                             });

  window_set_user_data(screen->window, screen);

  prv_load_departures(screen);

  LOG_DEBUG("Departures screen created");

  return screen;
}

void departures_screen_push(DeparturesScreen *screen) {
  LOG_DEBUG("Departures screen pushed");
  window_manager_push_window(screen->window);
}
