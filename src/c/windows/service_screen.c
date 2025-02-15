#include "service_screen.h"
#include "departures_screen.h"
#include "../data.h"
#include "../utils.h"
#include "../layers/spinner_layer.h"
#include "../layers/status_bar.h"
#include "../layers/menu_header.h"
#include "../layers/journey_item.h"
#include "../layers/service_summary.h"

static Window *s_window;
static StatusBarLayer *s_status_bar;
static MenuLayer *s_menu_layer;
static Layer *s_spinner_layer;
static TextLayer *s_error_layer;
static ServiceSummaryLayer *s_service_summary_layer;

#define ACTION_MENU_NUM_ITEMS 2
static ActionMenu *s_action_menu;
static ActionMenuLevel *s_root_level;
static uint8_t s_selected_calling_point_index = 0;

#define MAX_CALLING_POINT_COUNT 30
static struct CallingPointEntry s_calling_points[MAX_CALLING_POINT_COUNT];
static uint8_t s_calling_point_count = 0;
static uint8_t s_available_calling_points = 0;
static ServiceInfo s_service_info;

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
    pin_calling_point(s_service_info.serviceID, s_calling_points[s_selected_calling_point_index].crs, true);
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
  journey_item_draw(ctx, cell_layer, s_calling_points[index].crs == s_service_info.origin, &s_calling_points[index]);
#endif
}

static void menu_draw_header_callback(GContext *ctx, const Layer *cell_layer, uint16_t section_index, void *data)
{
  char combined_text[32];
  snprintf(combined_text, sizeof(combined_text), "%s -> %s", s_service_info.origin, s_service_info.destination);

  menu_section_header_draw(ctx, cell_layer, combined_text);
}

static int16_t menu_get_header_height_callback(MenuLayer *menu_layer, uint16_t section_index, void *data)
{
  return 20;
}

static void menu_select_click_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data)
{
  // If the menu is hidden, don't allow clicks
  if (layer_get_hidden(menu_layer_get_layer(s_menu_layer)))
  {
    return;
  }

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

static uint16_t get_origin_calling_point_index()
{
  if (s_available_calling_points == s_calling_point_count)
  {
    for (int i = 0; i < s_available_calling_points; i++)
    {
      if (strcmp(s_calling_points[i].crs, s_service_info.origin) == 0)
      {
        APP_LOG(APP_LOG_LEVEL_DEBUG, "Origin calling point %s = %s index: %d", s_calling_points[i].crs, s_service_info.origin, i);
        return i;
      }
    }
  }

  return 0;
}

static uint16_t get_destination_calling_point_index()
{
  if (s_available_calling_points == s_calling_point_count)
  {
    for (int i = 0; i < s_available_calling_points; i++)
    {
      if (strcmp(s_calling_points[i].crs, s_service_info.destination) == 0)
      {
        return i;
      }
    }
  }

  return s_available_calling_points - 1;
}

static CallingPointEntry *get_origin_calling_point()
{
  return &s_calling_points[get_origin_calling_point_index()];
}

static CallingPointEntry *get_destination_calling_point()
{
  return &s_calling_points[get_destination_calling_point_index()];
}

// Forward declare click config providers
static void menu_click_config_provider(void *context);
static void summary_click_config_provider(void *context);

// Displays the menu and binds the click handler
static void activate_menu()
{
  custom_status_bar_set_color(s_status_bar, GColorWhite);

  layer_set_hidden(s_service_summary_layer, true);
  layer_set_hidden(menu_layer_get_layer(s_menu_layer), false);
  layer_mark_dirty(menu_layer_get_layer(s_menu_layer));

  scroll_layer_set_callbacks(menu_layer_get_scroll_layer(s_menu_layer), (ScrollLayerCallbacks){
                                                                            .click_config_provider = menu_click_config_provider,
                                                                        });
  menu_layer_set_click_config_onto_window(s_menu_layer, s_window);
}

// Hides the menu and unbinds the click handler
static void deactivate_menu()
{
  layer_set_hidden(menu_layer_get_layer(s_menu_layer), true);
}

static void show_service_summary()
{
  CallingPointEntry *origin = get_origin_calling_point();
  uint16_t destination_index = get_destination_calling_point_index();
  CallingPointEntry *destination = &s_calling_points[destination_index];

  bool part_cancelled = destination_index + 1 < s_available_calling_points;

  char *reason = NULL;

  if (strlen(s_service_info.cancelReason) > 1)
  {
    char *prefix = part_cancelled ? "Part cancelled due to " : "Cancelled due to ";
    uint16_t total_length = strlen(prefix) + strlen(s_service_info.cancelReason) + 1;
    reason = malloc(total_length);

    strcpy(reason, prefix);
    strcat(reason, s_service_info.cancelReason);
  }
  else if (strlen(s_service_info.delayReason) > 1)
  {
    char *prefix = "Delayed due to ";
    uint16_t total_length = strlen(prefix) + strlen(s_service_info.delayReason) + 1;
    reason = malloc(total_length);

    strcpy(reason, prefix);
    strcat(reason, s_service_info.delayReason);
  }

  service_summary_set_data(
      s_service_summary_layer,
      origin->destination,
      destination->destination,
      s_service_info.operatorCode,
      origin->departureTime,
      reason,
      origin->platform);

  custom_status_bar_set_color(s_status_bar, service_summary_get_color(s_service_summary_layer));

  layer_set_hidden(s_service_summary_layer, false);
  layer_set_hidden(menu_layer_get_layer(s_menu_layer), true);
  layer_mark_dirty(s_service_summary_layer);

  window_set_click_config_provider(s_window, summary_click_config_provider);
}

static void menu_up_handler(ClickRecognizerRef recognizer, void *context)
{
  MenuIndex index = menu_layer_get_selected_index(s_menu_layer);
  if (index.row == 0)
  {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Going back to service summary");
    deactivate_menu();
    show_service_summary();
    return;
  }

  menu_layer_set_selected_next(s_menu_layer, true, MenuRowAlignCenter, true);
}

static void menu_down_handler(ClickRecognizerRef recognizer, void *context)
{
  menu_layer_set_selected_next(s_menu_layer, false, MenuRowAlignCenter, true);
}

static void menu_click_config_provider(void *context)
{
  window_single_click_subscribe(BUTTON_ID_UP, menu_up_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, menu_down_handler);
}

static void create_service_summary()
{
  GRect bounds = bounds_with_status_bar_no_padding(s_window);
  s_service_summary_layer = service_summary_init(bounds);
  layer_add_child(window_get_root_layer(s_window), s_service_summary_layer);
  layer_set_hidden(s_service_summary_layer, true);
}

static void window_down_handler(ClickRecognizerRef recognizer, void *context)
{
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Switching to calling point list");
  activate_menu();
}

static void summary_click_config_provider(void *context)
{
  window_single_click_subscribe(BUTTON_ID_DOWN, window_down_handler);
}

static void service_load_complete()
{
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Received all %d calling points", s_available_calling_points);

  COPY_STRING(s_service_info.destination, get_destination_calling_point()->crs);

  menu_layer_reload_data(s_menu_layer);

  show_service_summary();

  spinner_layer_deinit(s_spinner_layer);
}

static void no_service()
{
  layer_set_hidden(menu_layer_get_layer(s_menu_layer), true);
  spinner_layer_deinit(s_spinner_layer);

  s_error_layer = create_error_layer(s_window, "Service not found");
  layer_add_child(window_get_root_layer(s_window), text_layer_get_layer(s_error_layer));
}

static void service_info_callback(DictionaryIterator *iter)
{
  EXTRACT_TUPLE(iter, destination, destination);
  EXTRACT_TUPLE(iter, operatorCode, operatorCode);
  EXTRACT_TUPLE(iter, isCancelled, isCancelled);
  EXTRACT_TUPLE(iter, cancelReason, cancelReason);
  EXTRACT_TUPLE(iter, delayReason, delayReason);

  COPY_STRING(s_service_info.destination, destination);
  COPY_STRING(s_service_info.operatorCode, operatorCode);
  COPY_STRING(s_service_info.cancelReason, cancelReason);
  COPY_STRING(s_service_info.delayReason, delayReason);
  s_service_info.isCancelled = isCancelled;
}

static void calling_point_callback(DictionaryIterator *iter)
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

  EXTRACT_TUPLE(iter, locationName, locationName);
  EXTRACT_TUPLE(iter, time, time);
  EXTRACT_TUPLE(iter, platform, platform);
  EXTRACT_TUPLE(iter, crs, crs);
  EXTRACT_TUPLE(iter, skipped, skipped);

  COPY_STRING(s_calling_points[s_calling_point_count].destination, locationName);
  COPY_STRING(s_calling_points[s_calling_point_count].departureTime, time);
  COPY_STRING(s_calling_points[s_calling_point_count].platform, platform);
  COPY_STRING(s_calling_points[s_calling_point_count].crs, crs);
  s_calling_points[s_calling_point_count].skipped = skipped;

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

  set_service_info_callback(service_info_callback);
  set_calling_point_callback(calling_point_callback);
  request_service(s_service_info.serviceID);
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

  layer_set_hidden(menu_layer_get_layer(s_menu_layer), true);

  s_spinner_layer = spinner_layer_init(bounds);

  layer_add_child(window_layer, menu_layer_get_layer(s_menu_layer));
  layer_add_child(window_layer, s_spinner_layer);
  create_service_summary();
  layer_add_child(window_layer, status_bar_layer_get_layer(s_status_bar));
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Station screen initialized");
}

void service_window_unload(Window *window)
{
  service_screen_deinit();
}

void service_screen_init(char *service_id, char *origin)
{
  COPY_STRING(s_service_info.serviceID, service_id);
  COPY_STRING(s_service_info.origin, origin);

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Service screen initialized with service ID: %s and origin: %s", s_service_info.serviceID, s_service_info.origin);

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
