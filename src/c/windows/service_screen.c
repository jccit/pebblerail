#include "service_screen.h"

#include "../data.h"
#include "../layers/journey_item.h"
#include "../layers/menu_header.h"
#include "../layers/service_summary.h"
#include "../layers/spinner_layer.h"
#include "../layers/status_bar.h"
#include "../utils.h"
#include "departures_screen.h"

static Window *s_window;
static StatusBarLayer *s_status_bar;
static Layer *s_spinner_layer;
static TextLayer *s_error_layer;
static ServiceSummaryLayer *s_service_summary_layer;

#ifdef PBL_ROUND
#include "../layers/round_route_path.h"
static Layer *s_route_path_layer;
#else
static MenuLayer *s_menu_layer;
#endif

#define ACTION_MENU_NUM_ITEMS 2
static ActionMenu *s_action_menu;
static ActionMenuLevel *s_root_level;
static uint8_t s_selected_calling_point_index = 0;

#if defined(PBL_PLATFORM_APLITE)
// Aplite has limited RAM so restrict the number of calling points
#define MAX_CALLING_POINT_COUNT 30
#elif defined(PBL_ROUND)
// Round has limited screen space so can't really show all calling points
// TODO: Add a way to show more calling points
#define MAX_CALLING_POINT_COUNT 30
#else
// All other platforms (basalt, diorite, core devices) should have enough RAM to show more
#define MAX_CALLING_POINT_COUNT 50
#endif

static struct CallingPointEntry s_calling_points[MAX_CALLING_POINT_COUNT];
static uint8_t s_calling_point_count = 0;
static uint8_t s_available_calling_points = 0;
static ServiceInfo s_service_info;
static bool s_is_loading = false;
static bool s_is_menu_active = false;

static GRect s_menu_frame_start;

const int MENU_ANIMATION_DURATION = 150;

void load_service();

typedef enum { MENU_ACTION_VIEW_DEPARTURES = 1, MENU_ACTION_PIN = 2 } ServiceMenuAction;

static void action_performed_callback(ActionMenu *action_menu, const ActionMenuItem *action, void *context) {
  ServiceMenuAction selected_action = (ServiceMenuAction)action_menu_item_get_action_data(action);
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Selected action: %d", selected_action);

  if (selected_action == MENU_ACTION_VIEW_DEPARTURES) {
    departures_screen_init(s_calling_points[s_selected_calling_point_index].crs, s_calling_points[s_selected_calling_point_index].destination);
  } else if (selected_action == MENU_ACTION_PIN) {
    pin_calling_point(s_service_info.serviceID, s_calling_points[s_selected_calling_point_index].crs, true);
  }
}

#ifndef PBL_ROUND
// ------ MENU LAYER CALLBACKS ------

static uint16_t menu_get_num_sections_callback(MenuLayer *menu_layer, void *data) { return 1; }

static uint16_t menu_get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) { return s_calling_point_count; }

static int16_t menu_get_cell_height_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data) { return 46; }

static void menu_draw_row_callback(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data) {
  int index = cell_index->row;
  bool start = strcmp(s_calling_points[index].crs, s_service_info.origin) == 0;
  bool end = strcmp(s_calling_points[index].crs, s_service_info.destination) == 0;
  journey_item_draw(ctx, cell_layer, start, end, &s_calling_points[index]);
}

static void menu_draw_header_callback(GContext *ctx, const Layer *cell_layer, uint16_t section_index, void *data) {
  char combined_text[32];
  snprintf(combined_text, sizeof(combined_text), "%s -> %s", s_service_info.origin, s_service_info.destination);

  menu_section_header_draw(ctx, cell_layer, combined_text);
}

static int16_t menu_get_header_height_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) { return 20; }

static void menu_select_click_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data) {
  // If the menu is hidden, don't allow clicks
  if (layer_get_hidden(menu_layer_get_layer(s_menu_layer))) {
    return;
  }

  // Configure the ActionMenu Window about to be shown
  ActionMenuConfig config = (ActionMenuConfig){.root_level = s_root_level,
                                               .colors =
                                                   {
                                                       .background = GColorWhite,
                                                       .foreground = GColorBlack,
                                                   },
                                               .align = ActionMenuAlignCenter};

  // Show the ActionMenu
  s_selected_calling_point_index = cell_index->row;
  s_action_menu = action_menu_open(&config);
}

// ------ END MENU LAYER CALLBACKS ------
#endif

static uint16_t get_origin_calling_point_index() {
  if (s_available_calling_points == s_calling_point_count) {
    for (int i = 0; i < s_available_calling_points; i++) {
      if (strcmp(s_calling_points[i].crs, s_service_info.origin) == 0) {
        APP_LOG(APP_LOG_LEVEL_DEBUG, "Origin calling point %s = %s index: %d", s_calling_points[i].crs, s_service_info.origin, i);
        return i;
      }
    }
  }

  return 0;
}

static uint16_t get_destination_calling_point_index() {
  if (s_available_calling_points == s_calling_point_count) {
    for (int i = 0; i < s_available_calling_points; i++) {
      if (strcmp(s_calling_points[i].crs, s_service_info.destination) == 0) {
        return i;
      }
    }
  }

  return s_available_calling_points - 1;
}

static CallingPointEntry *get_origin_calling_point() { return &s_calling_points[get_origin_calling_point_index()]; }

static CallingPointEntry *get_destination_calling_point() { return &s_calling_points[get_destination_calling_point_index()]; }

// Forward declare click config providers
#ifndef PBL_ROUND
static void menu_click_config_provider(void *context);
#endif
static void summary_click_config_provider(void *context);

static GRect get_menu_offscreen_frame() {
  return GRect(s_menu_frame_start.origin.x, s_menu_frame_start.origin.y + s_menu_frame_start.size.h, s_menu_frame_start.size.w,
               s_menu_frame_start.size.h);
}

static void animate_menu_in() {
#ifdef PBL_ROUND
  Layer *menu_layer = s_route_path_layer;
#else
  Layer *menu_layer = menu_layer_get_layer(s_menu_layer);
#endif

  GRect start = get_menu_offscreen_frame();
  GRect end = s_menu_frame_start;

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Menu anim start: %d, %d, %d, %d", start.origin.x, start.origin.y, start.size.w, start.size.h);
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Menu anim end: %d, %d, %d, %d", end.origin.x, end.origin.y, end.size.w, end.size.h);

  PropertyAnimation *prop_anim = property_animation_create_layer_frame(menu_layer, &start, &end);
  Animation *anim = property_animation_get_animation(prop_anim);
  animation_set_curve(anim, AnimationCurveEaseOut);
  animation_set_duration(anim, MENU_ANIMATION_DURATION);
  animation_schedule(anim);

#ifdef PBL_ROUND
  layer_set_hidden(s_route_path_layer, false);
#else
  layer_set_hidden(menu_layer_get_layer(s_menu_layer), false);
#endif
}

static void menu_animated_out_callback(Animation *animation, bool finished, void *context) {
#ifdef PBL_ROUND
  layer_set_hidden(s_route_path_layer, true);
#else
  layer_set_hidden(menu_layer_get_layer(s_menu_layer), true);
#endif
}

static void animate_menu_out() {
#ifdef PBL_ROUND
  Layer *menu_layer = s_route_path_layer;
#else
  Layer *menu_layer = menu_layer_get_layer(s_menu_layer);
#endif

  GRect start = s_menu_frame_start;
  GRect end = get_menu_offscreen_frame();

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Menu anim start: %d, %d, %d, %d", start.origin.x, start.origin.y, start.size.w, start.size.h);
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Menu anim end: %d, %d, %d, %d", end.origin.x, end.origin.y, end.size.w, end.size.h);

  PropertyAnimation *prop_anim = property_animation_create_layer_frame(menu_layer, &start, &end);
  Animation *anim = property_animation_get_animation(prop_anim);
  animation_set_curve(anim, AnimationCurveEaseOut);
  animation_set_duration(anim, MENU_ANIMATION_DURATION);
  animation_set_handlers(anim,
                         (AnimationHandlers){
                             .stopped = menu_animated_out_callback,
                         },
                         NULL);
  animation_schedule(anim);
}

// Displays the menu and binds the click handler
static void activate_menu() {
  s_is_menu_active = true;
  custom_status_bar_set_color(s_status_bar, GColorWhite);

#ifndef PBL_ROUND
  scroll_layer_set_callbacks(menu_layer_get_scroll_layer(s_menu_layer), (ScrollLayerCallbacks){
                                                                            .click_config_provider = menu_click_config_provider,
                                                                        });
  menu_layer_set_click_config_onto_window(s_menu_layer, s_window);
#else
  layer_set_hidden(status_bar_layer_get_layer(s_status_bar), true);
#endif

  service_summary_animate_out(s_service_summary_layer);
  animate_menu_in();
}

static void deactivate_menu() {
  s_is_menu_active = false;

#ifdef PBL_ROUND
  layer_set_hidden(status_bar_layer_get_layer(s_status_bar), false);
#endif

  animate_menu_out();
  service_summary_animate_in(s_service_summary_layer);
}

static void show_service_summary() {
  CallingPointEntry *origin = get_origin_calling_point();
  uint16_t destination_index = get_destination_calling_point_index();
  CallingPointEntry *destination = &s_calling_points[destination_index];

  bool part_cancelled = destination_index + 1 < s_available_calling_points;

  char *reason = NULL;

  if (strlen(s_service_info.cancelReason) > 1) {
    char *prefix = part_cancelled ? "Part cancelled due to " : "Cancelled due to ";
    uint16_t total_length = strlen(prefix) + strlen(s_service_info.cancelReason) + 1;
    reason = malloc(total_length);

    strcpy(reason, prefix);
    strcat(reason, s_service_info.cancelReason);
  } else if (strlen(s_service_info.delayReason) > 1) {
    char *prefix = "Delayed due to ";
    uint16_t total_length = strlen(prefix) + strlen(s_service_info.delayReason) + 1;
    reason = malloc(total_length);

    strcpy(reason, prefix);
    strcat(reason, s_service_info.delayReason);
  }

  service_summary_set_data(s_service_summary_layer, origin->destination, destination->destination, s_service_info.operatorCode, origin->time, reason,
                           origin->platform, origin->state);

  custom_status_bar_set_color(s_status_bar, service_summary_get_color(s_service_summary_layer));

  layer_set_hidden(s_service_summary_layer, false);

  window_set_click_config_provider(s_window, summary_click_config_provider);
}

#ifndef PBL_ROUND
static void menu_up_handler(ClickRecognizerRef recognizer, void *context) {
  MenuIndex index = menu_layer_get_selected_index(s_menu_layer);
  if (index.row == 0) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Going back to service summary");
    deactivate_menu();
    show_service_summary();
    return;
  }

  menu_layer_set_selected_next(s_menu_layer, true, MenuRowAlignCenter, true);
}

static void menu_down_handler(ClickRecognizerRef recognizer, void *context) {
  // Don't allow switching to the the calling point list while loading
  if (s_is_loading) {
    return;
  }

  menu_layer_set_selected_next(s_menu_layer, false, MenuRowAlignCenter, true);
}

static void menu_select_handler(ClickRecognizerRef recognizer, void *context) {
  MenuIndex index = menu_layer_get_selected_index(s_menu_layer);
  menu_select_click_callback(s_menu_layer, &index, NULL);
}

static void menu_click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_UP, menu_up_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, menu_down_handler);
  window_single_click_subscribe(BUTTON_ID_SELECT, menu_select_handler);
}
#endif

static void create_service_summary() {
  GRect bounds = bounds_with_status_bar_no_padding(s_window);
  s_service_summary_layer = service_summary_init(bounds);
  layer_add_child(window_get_root_layer(s_window), s_service_summary_layer);
  layer_set_hidden(s_service_summary_layer, true);
}

static void window_double_select_handler(ClickRecognizerRef recognizer, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Reloading service data");
  load_service();
}

static void window_down_handler(ClickRecognizerRef recognizer, void *context) {
  if (!s_is_menu_active) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Switching to calling point list");
    activate_menu();
    return;
  }

#ifdef PBL_ROUND
  if (!round_route_next_calling_point(s_route_path_layer)) {
    return;
  }
#endif
}

static void window_up_handler(ClickRecognizerRef recognizer, void *context) {
  if (!s_is_menu_active) {
    return;
  }

#ifdef PBL_ROUND
  if (!round_route_previous_calling_point(s_route_path_layer)) {
    return;
  }
#endif

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Switching to service summary");
  deactivate_menu();
  show_service_summary();
}

static void summary_click_config_provider(void *context) {
#ifdef PBL_ROUND
  window_single_click_subscribe(BUTTON_ID_UP, window_up_handler);
#endif
  window_single_click_subscribe(BUTTON_ID_DOWN, window_down_handler);
  window_long_click_subscribe(BUTTON_ID_SELECT, 300, window_double_select_handler, NULL);
}

static void service_load_complete() {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Received all %d calling points", s_available_calling_points);
  s_is_loading = false;

  COPY_STRING(s_service_info.destination, get_destination_calling_point()->crs);

#ifndef PBL_ROUND
  menu_layer_reload_data(s_menu_layer);
#endif

  show_service_summary();

  layer_set_hidden(s_spinner_layer, true);

#ifdef PBL_ROUND
  round_route_path_set_data(s_route_path_layer, s_calling_points, s_available_calling_points, s_service_info.operatorCode);
#endif
}

static void no_service() {
#ifdef PBL_ROUND
  layer_set_hidden(s_route_path_layer, true);
#else
  layer_set_hidden(menu_layer_get_layer(s_menu_layer), true);
#endif
  layer_set_hidden(s_spinner_layer, true);

  s_error_layer = create_error_layer(s_window, "Service not found");
  layer_add_child(window_get_root_layer(s_window), text_layer_get_layer(s_error_layer));
}

static void service_info_callback(DictionaryIterator *iter) {
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

static void calling_point_callback(DictionaryIterator *iter) {
#ifndef PBL_ROUND
  layer_set_hidden(menu_layer_get_layer(s_menu_layer), true);
#endif

  Tuple *count_tuple = dict_find(iter, MESSAGE_KEY_count);
  if (!count_tuple) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "No departure data received");
    return;
  }

  uint8_t count = count_tuple->value->uint8;
  s_available_calling_points = count > MAX_CALLING_POINT_COUNT ? MAX_CALLING_POINT_COUNT : count;
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Set available calling points to %d", s_available_calling_points);

  if (s_available_calling_points == 0) {
    no_service();
    return;
  }

  if (s_calling_point_count >= s_available_calling_points) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Received more calling points than available: %d >= %d", s_calling_point_count, s_available_calling_points);
    return;
  }

  EXTRACT_TUPLE(iter, locationName, locationName);
  EXTRACT_TUPLE(iter, time, time);
  EXTRACT_TUPLE(iter, platform, platform);
  EXTRACT_TUPLE(iter, crs, crs);
  EXTRACT_INT(iter, callingPointState, state);

  COPY_STRING(s_calling_points[s_calling_point_count].destination, locationName);
  COPY_STRING(s_calling_points[s_calling_point_count].time, time);
  COPY_STRING(s_calling_points[s_calling_point_count].platform, platform);
  COPY_STRING(s_calling_points[s_calling_point_count].crs, crs);
  s_calling_points[s_calling_point_count].state = state;

  to_local_time(time, s_calling_points[s_calling_point_count].time);

  s_calling_point_count++;

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Received calling point %d: %s, %s, %s", s_calling_point_count,
          s_calling_points[s_calling_point_count - 1].destination, s_calling_points[s_calling_point_count - 1].time,
          s_calling_points[s_calling_point_count - 1].platform);

  if (s_calling_point_count == s_available_calling_points) {
    service_load_complete();
  }
}

void load_service() {
  if (s_is_loading) {
    return;
  }

  s_calling_point_count = 0;
  s_is_loading = true;

  layer_set_hidden(s_spinner_layer, false);
  layer_set_hidden(s_service_summary_layer, true);

  set_service_info_callback(service_info_callback);
  set_calling_point_callback(calling_point_callback);
  request_service(s_service_info.serviceID, s_service_info.origin);
}

static void init_action_menu() {
  s_root_level = action_menu_level_create(ACTION_MENU_NUM_ITEMS);

  action_menu_level_add_action(s_root_level, "View departures", action_performed_callback, (void *)MENU_ACTION_VIEW_DEPARTURES);
  action_menu_level_add_action(s_root_level, "Pin to timeline", action_performed_callback, (void *)MENU_ACTION_PIN);
}

void service_window_load(Window *window) {
  s_status_bar = custom_status_bar_layer_create();

  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  GRect bounds_status_bar = bounds_with_status_bar(window);

  s_spinner_layer = spinner_layer_init(bounds);

#ifdef PBL_ROUND
  s_menu_frame_start = bounds;
  s_route_path_layer = round_route_path_init(get_menu_offscreen_frame());
  layer_add_child(window_layer, s_route_path_layer);
#else
  s_menu_frame_start = bounds_status_bar;
  s_menu_layer = menu_layer_create(get_menu_offscreen_frame());

  menu_layer_set_callbacks(s_menu_layer, NULL,
                           (MenuLayerCallbacks){
                               .get_num_sections = menu_get_num_sections_callback,
                               .get_num_rows = menu_get_num_rows_callback,
                               .get_header_height = menu_get_header_height_callback,
                               .get_cell_height = menu_get_cell_height_callback,
                               .draw_row = menu_draw_row_callback,
                               .draw_header = menu_draw_header_callback,
                               .select_click = menu_select_click_callback,
                           });

  layer_set_hidden(menu_layer_get_layer(s_menu_layer), true);
  layer_add_child(window_layer, menu_layer_get_layer(s_menu_layer));
#endif

  layer_add_child(window_layer, s_spinner_layer);
  create_service_summary();
  layer_add_child(window_layer, status_bar_layer_get_layer(s_status_bar));
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Station screen initialized");
}

void service_window_unload(Window *window) { service_screen_deinit(); }

void service_screen_init(char *service_id, char *origin) {
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

void service_screen_deinit() {
  s_is_menu_active = false;
  s_is_loading = false;

  custom_status_bar_layer_destroy(s_status_bar);
#ifndef PBL_ROUND
  menu_layer_destroy(s_menu_layer);
#endif
  window_destroy(s_window);
}
