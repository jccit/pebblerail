#include "departures_screen.h"
#include "service_screen.h"
#include "../data.h"
#include "../utils.h"
#include "../layers/spinner_layer.h"
#include "../layers/status_bar.h"
#include "../layers/menu_header.h"

static Window *s_window;
static StatusBarLayer *s_status_bar;
static MenuLayer *s_menu_layer;
static Layer *s_spinner_layer;
static TextLayer *s_error_layer;
static char *s_crs;
static char *s_stationName;

#define ACTION_MENU_NUM_ITEMS 2
static ActionMenu *s_action_menu;
static ActionMenuLevel *s_root_level;
static uint8_t s_selected_departure_index = 0;

#define MAX_DEPARTURE_COUNT 10
static struct DepartureEntry s_departures[MAX_DEPARTURE_COUNT];
static uint8_t s_departure_count = 0;
static uint8_t s_available_departures = 0;

typedef enum
{
  MENU_ACTION_VIEW_STOPS = 1,
  MENU_ACTION_PIN = 2
} DepartureMenuAction;

static void action_performed_callback(ActionMenu *action_menu, const ActionMenuItem *action, void *context)
{
  DepartureMenuAction selected_action = (DepartureMenuAction)action_menu_item_get_action_data(action);
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Selected action: %d", selected_action);

  if (selected_action == MENU_ACTION_VIEW_STOPS)
  {
    service_screen_init(s_departures[s_selected_departure_index].serviceID);
  }
  else if (selected_action == MENU_ACTION_PIN)
  {
    pin_calling_point(s_departures[s_selected_departure_index].serviceID, s_crs, false);
  }
}

// ------ MENU LAYER CALLBACKS ------

static uint16_t menu_get_num_sections_callback(MenuLayer *menu_layer, void *data)
{
  return 1;
}

static uint16_t menu_get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *data)
{
  return s_departure_count;
}

static void menu_draw_row_callback(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data)
{
  char combined_text[32];
  int index = cell_index->row;
  char *platform_display = s_departures[index].platform;

  if (strcmp(platform_display, "un") == 0)
  {
    platform_display = "?";
  }

  snprintf(combined_text, sizeof(combined_text), "%s - Platform %s",
           s_departures[index].departureTime,
           platform_display);

  menu_cell_basic_draw(ctx, cell_layer, s_departures[index].destination, combined_text, NULL);
}

static void menu_draw_header_callback(GContext *ctx, const Layer *cell_layer, uint16_t section_index, void *data)
{
  menu_section_header_draw(ctx, cell_layer, s_stationName);
}

static int16_t menu_get_header_height_callback(MenuLayer *menu_layer, uint16_t section_index, void *data)
{
  return 20;
}

static void menu_select_click_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data)
{
  // Configure the ActionMenu Window about to be shown
  ActionMenuConfig config = (ActionMenuConfig){
      .root_level = s_root_level,
      .colors = {
          .background = GColorWhite,
          .foreground = GColorBlack,
      },
      .align = ActionMenuAlignCenter};

  // Show the ActionMenu
  s_selected_departure_index = cell_index->row;
  s_action_menu = action_menu_open(&config);
}

// ------ END MENU LAYER CALLBACKS ------

static void departures_load_complete()
{
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Received all %d departures", s_available_departures);

  menu_layer_reload_data(s_menu_layer);
  layer_set_hidden(menu_layer_get_layer(s_menu_layer), false);
  layer_mark_dirty(menu_layer_get_layer(s_menu_layer));

  spinner_layer_deinit(s_spinner_layer);
}

static void no_departures()
{
  departures_load_complete();
  layer_set_hidden(menu_layer_get_layer(s_menu_layer), true);

  s_error_layer = create_error_layer(s_window, "No departures found");
  layer_add_child(window_get_root_layer(s_window), text_layer_get_layer(s_error_layer));
}

static void departures_callback(DictionaryIterator *iter)
{
  layer_set_hidden(menu_layer_get_layer(s_menu_layer), true);

  Tuple *count_tuple = dict_find(iter, MESSAGE_KEY_count);
  if (!count_tuple)
  {
    APP_LOG(APP_LOG_LEVEL_ERROR, "No departure data received");
    return;
  }

  uint8_t count = count_tuple->value->uint8;
  s_available_departures = count > MAX_DEPARTURE_COUNT ? MAX_DEPARTURE_COUNT : count;
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Set available departures to %d", s_available_departures);

  if (s_available_departures == 0)
  {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "No departures received");
    no_departures();
    return;
  }

  Tuple *serviceID_tuple = dict_find(iter, MESSAGE_KEY_serviceID);
  if (!serviceID_tuple)
  {
    APP_LOG(APP_LOG_LEVEL_ERROR, "No serviceID data received");
    return;
  }

  char *serviceID = serviceID_tuple->value->cstring;

  Tuple *locationName_tuple = dict_find(iter, MESSAGE_KEY_locationName);
  if (!locationName_tuple)
  {
    APP_LOG(APP_LOG_LEVEL_ERROR, "No locationName data received");
    return;
  }
  char *locationName = locationName_tuple->value->cstring;

  Tuple *time_tuple = dict_find(iter, MESSAGE_KEY_time);
  if (!time_tuple)
  {
    APP_LOG(APP_LOG_LEVEL_ERROR, "No time data received");
    return;
  }
  char *time = time_tuple->value->cstring;

  Tuple *platform_tuple = dict_find(iter, MESSAGE_KEY_platform);
  if (!platform_tuple)
  {
    APP_LOG(APP_LOG_LEVEL_ERROR, "No platform data received");
    return;
  }
  char *platform = platform_tuple->value->cstring;

  strncpy(s_departures[s_departure_count].destination, locationName, sizeof(s_departures[s_departure_count].destination) - 1);
  s_departures[s_departure_count].destination[sizeof(s_departures[s_departure_count].destination) - 1] = '\0';

  strncpy(s_departures[s_departure_count].departureTime, time, sizeof(s_departures[s_departure_count].departureTime) - 1);
  s_departures[s_departure_count].departureTime[sizeof(s_departures[s_departure_count].departureTime) - 1] = '\0';

  strncpy(s_departures[s_departure_count].platform, platform, sizeof(s_departures[s_departure_count].platform) - 1);
  s_departures[s_departure_count].platform[sizeof(s_departures[s_departure_count].platform) - 1] = '\0';

  strncpy(s_departures[s_departure_count].serviceID, serviceID, sizeof(s_departures[s_departure_count].serviceID) - 1);
  s_departures[s_departure_count].serviceID[sizeof(s_departures[s_departure_count].serviceID) - 1] = '\0';

  s_departure_count++;

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Received departure %d: %s, %s, %s, %s", s_departure_count, s_departures[s_departure_count - 1].serviceID, s_departures[s_departure_count - 1].destination, s_departures[s_departure_count - 1].departureTime, s_departures[s_departure_count - 1].platform);

  if (s_departure_count == s_available_departures)
  {
    departures_load_complete();
  }
}

void load_departures()
{
  s_departure_count = 0;

  set_departures_callback(departures_callback);
  request_departures(s_crs);
}

void departures_window_load(Window *window)
{
  s_status_bar = custom_status_bar_layer_create();

  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  GRect bounds_status_bar = bounds_with_status_bar(window);
  s_menu_layer = menu_layer_create(bounds_status_bar);

  menu_layer_set_callbacks(s_menu_layer, NULL, (MenuLayerCallbacks){
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
}

static void init_action_menu()
{
  s_root_level = action_menu_level_create(ACTION_MENU_NUM_ITEMS);

  action_menu_level_add_action(s_root_level, "View stops", action_performed_callback, (void *)MENU_ACTION_VIEW_STOPS);
  action_menu_level_add_action(s_root_level, "Pin to timeline", action_performed_callback, (void *)MENU_ACTION_PIN);
}

void departures_window_unload(Window *window)
{
  departures_screen_deinit();
}

void departures_screen_init(char *crs, char *stationName)
{
  s_crs = crs;
  s_stationName = stationName;
  s_window = window_create();
  window_set_window_handlers(s_window, (WindowHandlers){
                                           .load = departures_window_load,
                                           .unload = departures_window_unload,
                                       });
  const bool animated = true;
  window_stack_push(s_window, animated);

  load_departures();
  init_action_menu();
}

void departures_screen_deinit()
{
  custom_status_bar_layer_destroy(s_status_bar);
  menu_layer_destroy(s_menu_layer);
  window_destroy(s_window);
}
