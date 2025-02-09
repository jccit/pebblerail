#include "service_screen.h"
#include "departures_screen.h"
#include "../data.h"
#include "../utils.h"
#include "../layers/spinner_layer.h"
#include "../layers/status_bar.h"
#include "../layers/menu_header.h"
#include "../layers/journey_item.h"

static Window *s_window;
static StatusBarLayer *s_status_bar;
static MenuLayer *s_menu_layer;
static Layer *s_spinner_layer;
static TextLayer *s_error_layer;
static char *s_service_id;
static char *s_origin;
static char *s_destination;

#define ACTION_MENU_NUM_ITEMS 2
static ActionMenu *s_action_menu;
static ActionMenuLevel *s_root_level;
static uint8_t s_selected_calling_point_index = 0;

#define MAX_CALLING_POINT_COUNT 30
static struct CallingPointEntry s_calling_points[MAX_CALLING_POINT_COUNT];
static uint8_t s_calling_point_count = 0;
static uint8_t s_available_calling_points = 0;

typedef enum
{
  MENU_ACTION_VIEW_DEPARTURES = 1,
  MENU_ACTION_PIN = 2
} ServiceMenuAction;

static void action_performed_callback(ActionMenu *action_menu, const ActionMenuItem *action, void *context)
{
  ServiceMenuAction selected_action = (ServiceMenuAction)action_menu_item_get_action_data(action);
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Selected action: %d", selected_action);

  if (selected_action == MENU_ACTION_VIEW_DEPARTURES)
  {
    departures_screen_init(s_calling_points[s_selected_calling_point_index].crs, s_calling_points[s_selected_calling_point_index].destination);
  }
  else if (selected_action == MENU_ACTION_PIN)
  {
    pin_calling_point(s_service_id, s_calling_points[s_selected_calling_point_index].crs, true);
  }
}

// ------ MENU LAYER CALLBACKS ------

static uint16_t menu_get_num_sections_callback(MenuLayer *menu_layer, void *data)
{
  return 1;
}

static uint16_t menu_get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *data)
{
  return s_calling_point_count;
}

static void menu_draw_row_callback(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data)
{
#ifdef PBL_ROUND
  int index = cell_index->row;
  menu_cell_basic_draw(ctx, cell_layer, s_calling_points[index].destination, s_calling_points[index].departureTime, NULL);
#else
  int index = cell_index->row;
  journey_item_draw(ctx, cell_layer, s_calling_points[index].crs == s_origin, &s_calling_points[index]);
#endif
}

static void menu_draw_header_callback(GContext *ctx, const Layer *cell_layer, uint16_t section_index, void *data)
{
  char combined_text[32];
  snprintf(combined_text, sizeof(combined_text), "%s -> %s", s_origin, s_destination);

  menu_section_header_draw(ctx, cell_layer, combined_text);
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
  s_selected_calling_point_index = cell_index->row;
  s_action_menu = action_menu_open(&config);
}

// ------ END MENU LAYER CALLBACKS ------

static void service_load_complete()
{
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Received all %d calling points", s_available_calling_points);

  s_origin = s_calling_points[0].crs;
  s_destination = s_calling_points[s_available_calling_points - 1].crs;

  menu_layer_reload_data(s_menu_layer);
  layer_set_hidden(menu_layer_get_layer(s_menu_layer), false);
  layer_mark_dirty(menu_layer_get_layer(s_menu_layer));

  spinner_layer_deinit(s_spinner_layer);
}

static void no_service()
{
  layer_set_hidden(menu_layer_get_layer(s_menu_layer), true);
  spinner_layer_deinit(s_spinner_layer);

  s_error_layer = create_error_layer(s_window, "Service not found");
  layer_add_child(window_get_root_layer(s_window), text_layer_get_layer(s_error_layer));
}

static void service_callback(DictionaryIterator *iter)
{
  layer_set_hidden(menu_layer_get_layer(s_menu_layer), true);

  Tuple *count_tuple = dict_find(iter, MESSAGE_KEY_count);
  if (!count_tuple)
  {
    APP_LOG(APP_LOG_LEVEL_ERROR, "No departure data received");
    return;
  }

  uint8_t count = count_tuple->value->uint8;
  s_available_calling_points = count > MAX_CALLING_POINT_COUNT ? MAX_CALLING_POINT_COUNT : count;
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Set available calling points to %d", s_available_calling_points);

  if (s_available_calling_points == 0)
  {
    no_service();
    return;
  }

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

  Tuple *crs_tuple = dict_find(iter, MESSAGE_KEY_crs);
  if (!crs_tuple)
  {
    APP_LOG(APP_LOG_LEVEL_ERROR, "No crs data received");
    return;
  }
  char *crs = crs_tuple->value->cstring;

  strncpy(s_calling_points[s_calling_point_count].destination, locationName, sizeof(s_calling_points[s_calling_point_count].destination) - 1);
  s_calling_points[s_calling_point_count].destination[sizeof(s_calling_points[s_calling_point_count].destination) - 1] = '\0';

  strncpy(s_calling_points[s_calling_point_count].departureTime, time, sizeof(s_calling_points[s_calling_point_count].departureTime) - 1);
  s_calling_points[s_calling_point_count].departureTime[sizeof(s_calling_points[s_calling_point_count].departureTime) - 1] = '\0';

  strncpy(s_calling_points[s_calling_point_count].platform, platform, sizeof(s_calling_points[s_calling_point_count].platform) - 1);
  s_calling_points[s_calling_point_count].platform[sizeof(s_calling_points[s_calling_point_count].platform) - 1] = '\0';

  strncpy(s_calling_points[s_calling_point_count].crs, crs, sizeof(s_calling_points[s_calling_point_count].crs) - 1);
  s_calling_points[s_calling_point_count].crs[sizeof(s_calling_points[s_calling_point_count].crs) - 1] = '\0';

  s_calling_point_count++;

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Received calling point %d: %s, %s, %s", s_calling_point_count, s_calling_points[s_calling_point_count - 1].destination, s_calling_points[s_calling_point_count - 1].departureTime, s_calling_points[s_calling_point_count - 1].platform);

  if (s_calling_point_count == s_available_calling_points)
  {
    service_load_complete();
  }
}

void load_service()
{
  s_calling_point_count = 0;

  set_service_callback(service_callback);
  request_service(s_service_id);
}

static void init_action_menu()
{
  s_root_level = action_menu_level_create(ACTION_MENU_NUM_ITEMS);

  action_menu_level_add_action(s_root_level, "View departures", action_performed_callback, (void *)MENU_ACTION_VIEW_DEPARTURES);
  action_menu_level_add_action(s_root_level, "Pin to timeline", action_performed_callback, (void *)MENU_ACTION_PIN);
}

void service_window_load(Window *window)
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

void service_window_unload(Window *window)
{
  service_screen_deinit();
}

void service_screen_init(char *service_id)
{
  s_service_id = service_id;
  s_window = window_create();
  window_set_window_handlers(s_window, (WindowHandlers){
                                           .load = service_window_load,
                                           .unload = service_window_unload,
                                       });
  const bool animated = true;
  window_stack_push(s_window, animated);

  load_service();
  init_action_menu();
}

void service_screen_deinit()
{
  custom_status_bar_layer_destroy(s_status_bar);
  menu_layer_destroy(s_menu_layer);
  window_destroy(s_window);
}
