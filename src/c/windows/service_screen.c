#include "service_screen.h"

#include "../data.h"
#include "../layers/journey_item.h"
#include "../layers/menu_header.h"
#include "../layers/service_summary.h"
#include "../layers/spinner_layer.h"
#include "../layers/status_bar.h"
#include "../utils.h"
#include "../window_manager.h"
#include "departures_screen.h"

#ifdef PBL_ROUND
#include "../layers/round_route_path.h"
#endif

#define ACTION_MENU_NUM_ITEMS 2

#if defined(PBL_PLATFORM_APLITE)
// Aplite has limited RAM so restrict the number of calling points
#define MAX_CALLING_POINT_COUNT 20
#elif defined(PBL_ROUND)
// Round has limited screen space so can't really show all calling points
// TODO: Add a way to show more calling points
#define MAX_CALLING_POINT_COUNT 30
#else
// All other platforms (basalt, diorite, core devices) should have enough RAM to show more
#define MAX_CALLING_POINT_COUNT 50
#endif

struct ServiceScreen {
  Window *window;
  StatusBarLayer *status_bar;
  Layer *spinner_layer;
  TextLayer *error_layer;
  ServiceSummaryLayer *service_summary_layer;

  ActionMenu *action_menu;
  ActionMenuLevel *root_level;

  GRect menu_frame_start;
  PropertyAnimation *menu_prop_anim;
  Animation *menu_anim;

  struct CallingPointEntry calling_points[MAX_CALLING_POINT_COUNT];
  uint8_t selected_calling_point_index;
  uint8_t calling_point_count;
  uint8_t available_calling_points;
  ServiceInfo service_info;
  bool load_complete;
  bool is_loading;
  bool is_menu_active;

#ifdef PBL_ROUND
  Layer *route_path_layer;
#else
  MenuLayer *menu_layer;
#endif
};

const int MENU_ANIMATION_DURATION = 150;

typedef enum { MENU_ACTION_VIEW_DEPARTURES = 1, MENU_ACTION_PIN = 2 } ServiceMenuAction;

void prv_load_service(ServiceScreen *screen);

static void action_performed_callback(ActionMenu *action_menu, const ActionMenuItem *action, void *context) {
  ServiceScreen *screen = context;
  ServiceMenuAction selected_action = (ServiceMenuAction)action_menu_item_get_action_data(action);
  LOG_DEBUG("Selected action: %d", selected_action);

  if (selected_action == MENU_ACTION_VIEW_DEPARTURES) {
    DeparturesScreen *departures_screen = departures_screen_create(screen->calling_points[screen->selected_calling_point_index].crs,
                                                                   screen->calling_points[screen->selected_calling_point_index].destination);
    departures_screen_push(departures_screen);
  } else if (selected_action == MENU_ACTION_PIN) {
    pin_calling_point(screen->service_info.serviceID, screen->calling_points[screen->selected_calling_point_index].crs, true);
  }
}

#ifndef PBL_ROUND
// ------ MENU LAYER CALLBACKS ------

static uint16_t menu_get_num_sections_callback(MenuLayer *menu_layer, void *data) { return 1; }

static uint16_t menu_get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) {
  ServiceScreen *screen = data;
  return screen->available_calling_points;
}

static int16_t menu_get_cell_height_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data) { return 46; }

static void menu_draw_row_callback(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data) {
  ServiceScreen *screen = data;

  int index = cell_index->row;
  bool start = strcmp(screen->calling_points[index].crs, screen->service_info.origin) == 0;
  bool end = strcmp(screen->calling_points[index].crs, screen->service_info.destination) == 0;
  journey_item_draw(ctx, cell_layer, start, end, &screen->calling_points[index]);
}

static void menu_draw_header_callback(GContext *ctx, const Layer *cell_layer, uint16_t section_index, void *data) {
  ServiceScreen *screen = data;

  char combined_text[32];
  snprintf(combined_text, sizeof(combined_text), "%s -> %s", screen->service_info.origin, screen->service_info.destination);

  menu_section_header_draw(ctx, cell_layer, combined_text);
}

static int16_t menu_get_header_height_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) { return 20; }

static void service_action_menu_close_callback(ActionMenu *menu, const ActionMenuItem *performed_action, void *context) {
  ServiceScreen *screen = context;
  // action_menu_hierarchy_destroy(action_menu_get_root_level(screen->action_menu), NULL, NULL);
}

static void menu_select_click_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data) {
  ServiceScreen *screen = data;

  // If the menu is hidden, don't allow clicks
  if (layer_get_hidden(menu_layer_get_layer(screen->menu_layer))) {
    return;
  }

  // Configure the ActionMenu Window about to be shown
  ActionMenuConfig config = (ActionMenuConfig){.root_level = screen->root_level,
                                               .colors =
                                                   {
                                                       .background = GColorWhite,
                                                       .foreground = GColorBlack,
                                                   },
                                               .align = ActionMenuAlignCenter,
                                               .did_close = service_action_menu_close_callback,
                                               .context = screen};

  // Show the ActionMenu
  screen->selected_calling_point_index = cell_index->row;
  LOG_DEBUG("Opening action menu: %p", screen->action_menu);
  screen->action_menu = action_menu_open(&config);
  LOG_DEBUG("Action menu opened: %p", screen->action_menu);
}

// ------ END MENU LAYER CALLBACKS ------
#endif

static uint16_t get_origin_calling_point_index(ServiceScreen *screen) {
  if (screen->available_calling_points == screen->calling_point_count) {
    for (int i = 0; i < screen->available_calling_points; i++) {
      if (strcmp(screen->calling_points[i].crs, screen->service_info.origin) == 0) {
        LOG_DEBUG_VERBOSE("Origin calling point %s = %s index: %d", screen->calling_points[i].crs, screen->service_info.origin, i);
        return i;
      }
    }
  }

  return 0;
}

static uint16_t get_destination_calling_point_index(ServiceScreen *screen) {
  if (screen->available_calling_points == screen->calling_point_count) {
    for (int i = 0; i < screen->available_calling_points; i++) {
      if (strcmp(screen->calling_points[i].crs, screen->service_info.destination) == 0) {
        return i;
      }
    }
  }

  return screen->available_calling_points - 1;
}

static CallingPointEntry *get_origin_calling_point(ServiceScreen *screen) { return &screen->calling_points[get_origin_calling_point_index(screen)]; }

static CallingPointEntry *get_destination_calling_point(ServiceScreen *screen) {
  return &screen->calling_points[get_destination_calling_point_index(screen)];
}

// Forward declare click config providers
#ifndef PBL_ROUND
static void menu_click_config_provider(void *context);
#endif
static void summary_click_config_provider(void *context);

static GRect get_menu_offscreen_frame(ServiceScreen *screen) {
  return GRect(screen->menu_frame_start.origin.x, screen->menu_frame_start.origin.y + screen->menu_frame_start.size.h,
               screen->menu_frame_start.size.w, screen->menu_frame_start.size.h);
}

static void free_menu_anim(ServiceScreen *screen) {
  if (screen->menu_anim != NULL) {
    animation_destroy(screen->menu_anim);
  }
  if (screen->menu_prop_anim != NULL) {
    property_animation_destroy(screen->menu_prop_anim);
  }
  screen->menu_prop_anim = NULL;
  screen->menu_anim = NULL;
}

static void animate_menu_in(ServiceScreen *screen) {
#ifdef PBL_ROUND
  Layer *menu_layer = screen->route_path_layer;
#else
  Layer *menu_layer = menu_layer_get_layer(screen->menu_layer);
#endif

  GRect start = get_menu_offscreen_frame(screen);
  GRect end = screen->menu_frame_start;

  LOG_DEBUG_VERBOSE("Menu anim start: %d, %d, %d, %d", start.origin.x, start.origin.y, start.size.w, start.size.h);
  LOG_DEBUG_VERBOSE("Menu anim end: %d, %d, %d, %d", end.origin.x, end.origin.y, end.size.w, end.size.h);

  free_menu_anim(screen);

  screen->menu_prop_anim = property_animation_create_layer_frame(menu_layer, &start, &end);
  screen->menu_anim = property_animation_get_animation(screen->menu_prop_anim);
  animation_set_curve(screen->menu_anim, AnimationCurveEaseOut);
  animation_set_duration(screen->menu_anim, MENU_ANIMATION_DURATION);
  animation_schedule(screen->menu_anim);

#ifdef PBL_ROUND
  layer_set_hidden(screen->route_path_layer, false);
#else
  layer_set_hidden(menu_layer_get_layer(screen->menu_layer), false);
#endif
}

static void menu_animated_out_callback(Animation *animation, bool finished, void *context) {
  ServiceScreen *screen = context;

#ifdef PBL_ROUND
  layer_set_hidden(screen->route_path_layer, true);
#else
  layer_set_hidden(menu_layer_get_layer(screen->menu_layer), true);
#endif
}

static void animate_menu_out(ServiceScreen *screen) {
#ifdef PBL_ROUND
  Layer *menu_layer = screen->route_path_layer;
#else
  Layer *menu_layer = menu_layer_get_layer(screen->menu_layer);
#endif

  GRect start = screen->menu_frame_start;
  GRect end = get_menu_offscreen_frame(screen);

  LOG_DEBUG_VERBOSE("Menu anim start: %d, %d, %d, %d", start.origin.x, start.origin.y, start.size.w, start.size.h);
  LOG_DEBUG_VERBOSE("Menu anim end: %d, %d, %d, %d", end.origin.x, end.origin.y, end.size.w, end.size.h);

  free_menu_anim(screen);

  screen->menu_prop_anim = property_animation_create_layer_frame(menu_layer, &start, &end);
  screen->menu_anim = property_animation_get_animation(screen->menu_prop_anim);
  animation_set_curve(screen->menu_anim, AnimationCurveEaseOut);
  animation_set_duration(screen->menu_anim, MENU_ANIMATION_DURATION);
  animation_set_handlers(screen->menu_anim,
                         (AnimationHandlers){
                             .stopped = menu_animated_out_callback,
                         },
                         screen);
  animation_schedule(screen->menu_anim);
}

static void prv_init_service_action_menu(ServiceScreen *screen) {
  LOG_DEBUG_VERBOSE("Creating action menu root level");

  screen->root_level = action_menu_level_create(ACTION_MENU_NUM_ITEMS);

  action_menu_level_add_action(screen->root_level, "View departures", action_performed_callback, (void *)MENU_ACTION_VIEW_DEPARTURES);
  action_menu_level_add_action(screen->root_level, "Pin to timeline", action_performed_callback, (void *)MENU_ACTION_PIN);
}

// Displays the menu and binds the click handler
static void activate_menu(ServiceScreen *screen, bool animated) {
  screen->is_menu_active = true;
  custom_status_bar_set_color(screen->status_bar, GColorWhite);

#ifndef PBL_ROUND
  ScrollLayer *scroll_layer = menu_layer_get_scroll_layer(screen->menu_layer);
  scroll_layer_set_callbacks(scroll_layer, (ScrollLayerCallbacks){
                                               .click_config_provider = menu_click_config_provider,
                                           });
  menu_layer_set_click_config_onto_window(screen->menu_layer, screen->window);
#else
  layer_set_hidden(status_bar_layer_get_layer(screen->status_bar), true);
#endif

  service_summary_animate_out(screen->service_summary_layer);

  if (animated) {
    animate_menu_in(screen);
  } else {
#ifndef PBL_ROUND
    layer_set_hidden(menu_layer_get_layer(screen->menu_layer), false);
    layer_set_frame(menu_layer_get_layer(screen->menu_layer), screen->menu_frame_start);
#endif
  }
}

static void deactivate_menu(ServiceScreen *screen) {
  screen->is_menu_active = false;

#ifdef PBL_ROUND
  layer_set_hidden(status_bar_layer_get_layer(screen->status_bar), false);
#endif

  animate_menu_out(screen);
  service_summary_animate_in(screen->service_summary_layer);
}

static void show_service_summary(ServiceScreen *screen) {
  CallingPointEntry *origin = get_origin_calling_point(screen);
  uint16_t destination_index = get_destination_calling_point_index(screen);
  CallingPointEntry *destination = &screen->calling_points[destination_index];

  bool part_cancelled = destination_index + 1 < screen->available_calling_points;

  char *reason = NULL;

  if (strlen(screen->service_info.cancelReason) > 1) {
    char *prefix = part_cancelled ? "Part cancelled due to " : "Cancelled due to ";
    uint16_t total_length = strlen(prefix) + strlen(screen->service_info.cancelReason) + 1;
    reason = wm_alloc(total_length);

    strcpy(reason, prefix);
    strcat(reason, screen->service_info.cancelReason);
  } else if (strlen(screen->service_info.delayReason) > 1) {
    char *prefix = "Delayed due to ";
    uint16_t total_length = strlen(prefix) + strlen(screen->service_info.delayReason) + 1;
    reason = wm_alloc(total_length);

    strcpy(reason, prefix);
    strcat(reason, screen->service_info.delayReason);
  }

  service_summary_set_data(screen->service_summary_layer, origin->destination, destination->destination, screen->service_info.operatorCode,
                           origin->time, reason, origin->platform, origin->state);

  custom_status_bar_set_color(screen->status_bar, service_summary_get_color(screen->service_summary_layer));

  layer_set_hidden(screen->service_summary_layer, false);

  window_set_click_config_provider_with_context(screen->window, summary_click_config_provider, screen);
}

#ifndef PBL_ROUND
static void menu_up_handler(ClickRecognizerRef recognizer, void *context) {
  ServiceScreen *screen = top_window_as_service_screen();
  MenuIndex index = menu_layer_get_selected_index(screen->menu_layer);

  if (index.row == 0) {
    LOG_DEBUG("Going back to service summary");
    deactivate_menu(screen);
    show_service_summary(screen);
    return;
  }

  menu_layer_set_selected_next(screen->menu_layer, true, MenuRowAlignCenter, true);
}

static void menu_down_handler(ClickRecognizerRef recognizer, void *context) {
  ServiceScreen *screen = top_window_as_service_screen();
  // Don't allow switching to the the calling point list while loading
  if (screen->is_loading) {
    return;
  }

  menu_layer_set_selected_next(screen->menu_layer, false, MenuRowAlignCenter, true);
}

static void menu_select_handler(ClickRecognizerRef recognizer, void *context) {
  ServiceScreen *screen = top_window_as_service_screen();

  MenuIndex index = menu_layer_get_selected_index(screen->menu_layer);
  menu_select_click_callback(screen->menu_layer, &index, screen);
}

static void menu_click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_UP, menu_up_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, menu_down_handler);
  window_single_click_subscribe(BUTTON_ID_SELECT, menu_select_handler);
}
#endif

static void window_double_select_handler(ClickRecognizerRef recognizer, void *context) {
  ServiceScreen *screen = context;

  LOG_DEBUG("Reloading service data");
  prv_load_service(screen);
}

static void window_down_handler(ClickRecognizerRef recognizer, void *context) {
  ServiceScreen *screen = context;

  if (!screen->is_menu_active) {
    LOG_DEBUG("Switching to calling point list");
    activate_menu(screen, true);
    return;
  }

#ifdef PBL_ROUND
  if (!round_route_next_calling_point(screen->route_path_layer)) {
    return;
  }
#endif
}

static void window_up_handler(ClickRecognizerRef recognizer, void *context) {
  ServiceScreen *screen = context;

  if (!screen->is_menu_active) {
    return;
  }

#ifdef PBL_ROUND
  if (!round_route_previous_calling_point(screen->route_path_layer)) {
    return;
  }
#endif

  LOG_DEBUG("Switching to service summary");
  deactivate_menu(screen);
  show_service_summary(screen);
}

static void summary_click_config_provider(void *context) {
#ifdef PBL_ROUND
  window_single_click_subscribe(BUTTON_ID_UP, window_up_handler);
#endif
  window_single_click_subscribe(BUTTON_ID_DOWN, window_down_handler);
  window_long_click_subscribe(BUTTON_ID_SELECT, 300, window_double_select_handler, NULL);
}

static void service_load_complete(ServiceScreen *screen) {
  LOG_DEBUG("Received all %d calling points", screen->available_calling_points);
  screen->is_loading = false;
  screen->load_complete = true;

  COPY_STRING(screen->service_info.destination, get_destination_calling_point(screen)->crs);

#ifndef PBL_ROUND
  menu_layer_reload_data(screen->menu_layer);
#endif

  show_service_summary(screen);

  layer_set_hidden(screen->spinner_layer, true);

#ifdef PBL_ROUND
  round_route_path_set_data(screen->route_path_layer, screen->calling_points, screen->available_calling_points, screen->service_info.operatorCode);
#endif

  if (screen->is_menu_active) {
    activate_menu(screen, false);
  }
}

static void no_service(ServiceScreen *screen) {
#ifdef PBL_ROUND
  layer_set_hidden(screen->route_path_layer, true);
#else
  layer_set_hidden(menu_layer_get_layer(screen->menu_layer), true);
#endif
  layer_set_hidden(screen->spinner_layer, true);

  screen->error_layer = create_error_layer(screen->window, "Service not found");
  layer_add_child(window_get_root_layer(screen->window), text_layer_get_layer(screen->error_layer));
}

static void service_info_callback(DictionaryIterator *iter, void *context) {
  ServiceScreen *screen = context;

  EXTRACT_TUPLE(iter, destination, destination);
  EXTRACT_TUPLE(iter, operatorCode, operatorCode);
  EXTRACT_TUPLE(iter, isCancelled, isCancelled);
  EXTRACT_TUPLE(iter, cancelReason, cancelReason);
  EXTRACT_TUPLE(iter, delayReason, delayReason);

  COPY_STRING(screen->service_info.destination, destination);
  COPY_STRING(screen->service_info.operatorCode, operatorCode);
  COPY_STRING(screen->service_info.cancelReason, cancelReason);
  COPY_STRING(screen->service_info.delayReason, delayReason);
  screen->service_info.isCancelled = isCancelled;
}

static void calling_point_callback(DictionaryIterator *iter, void *context) {
  ServiceScreen *screen = context;

#ifndef PBL_ROUND
  layer_set_hidden(menu_layer_get_layer(screen->menu_layer), true);
#endif

  Tuple *count_tuple = dict_find(iter, MESSAGE_KEY_count);
  if (!count_tuple) {
    LOG_ERROR("No departure data received");
    return;
  }

  uint8_t count = count_tuple->value->uint8;
  screen->available_calling_points = count > MAX_CALLING_POINT_COUNT ? MAX_CALLING_POINT_COUNT : count;
  LOG_DEBUG("Set available calling points to %d", screen->available_calling_points);

  if (screen->available_calling_points == 0) {
    no_service(screen);
    return;
  }

  if (screen->calling_point_count >= screen->available_calling_points) {
    LOG_DEBUG("Received more calling points than available: %d >= %d", screen->calling_point_count, screen->available_calling_points);
    return;
  }

  EXTRACT_TUPLE(iter, locationName, locationName);
  EXTRACT_TUPLE(iter, time, time);
  EXTRACT_TUPLE(iter, platform, platform);
  EXTRACT_TUPLE(iter, crs, crs);
  EXTRACT_INT(iter, callingPointState, state);

  COPY_STRING(screen->calling_points[screen->calling_point_count].destination, locationName);
  COPY_STRING(screen->calling_points[screen->calling_point_count].time, time);
  COPY_STRING(screen->calling_points[screen->calling_point_count].platform, platform);
  COPY_STRING(screen->calling_points[screen->calling_point_count].crs, crs);
  screen->calling_points[screen->calling_point_count].state = state;

  to_local_time(time, screen->calling_points[screen->calling_point_count].time);

  screen->calling_point_count++;

  LOG_DEBUG_VERBOSE("Received calling point %d: %s, %s, %s", screen->calling_point_count,
                    screen->calling_points[screen->calling_point_count - 1].destination, screen->calling_points[screen->calling_point_count - 1].time,
                    screen->calling_points[screen->calling_point_count - 1].platform);

  if (screen->calling_point_count == screen->available_calling_points) {
    service_load_complete(screen);
  }
}

void prv_load_service(ServiceScreen *screen) {
  if (screen->is_loading) {
    return;
  }

  LOG_DEBUG("Loading service %s from %s", screen->service_info.serviceID, screen->service_info.origin);

  screen->calling_point_count = 0;
  screen->is_loading = true;
  screen->is_menu_active = false;

  set_service_info_callback(service_info_callback, screen);
  set_calling_point_callback(calling_point_callback, screen);
  request_service(screen->service_info.serviceID, screen->service_info.origin);
}

void service_window_appear(Window *window) {
  LOG_DEBUG("Service screen appearing");

  ServiceScreen *screen = window_get_user_data(window);
  screen->status_bar = custom_status_bar_layer_create();

  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  GRect bounds_status_bar = bounds_with_status_bar(window);

  screen->spinner_layer = spinner_layer_init(bounds);

#ifdef PBL_ROUND
  screen->menu_frame_start = bounds;
  screen->route_path_layer = round_route_path_init(get_menu_offscreen_frame(screen));
  layer_add_child(window_layer, screen->route_path_layer);
#else
  screen->menu_frame_start = bounds_status_bar;
  screen->menu_layer = menu_layer_create(get_menu_offscreen_frame(screen));

  menu_layer_set_callbacks(screen->menu_layer, screen,
                           (MenuLayerCallbacks){
                               .get_num_sections = menu_get_num_sections_callback,
                               .get_num_rows = menu_get_num_rows_callback,
                               .get_header_height = menu_get_header_height_callback,
                               .get_cell_height = menu_get_cell_height_callback,
                               .draw_row = menu_draw_row_callback,
                               .draw_header = menu_draw_header_callback,
                               .select_click = menu_select_click_callback,
                           });

  layer_set_hidden(menu_layer_get_layer(screen->menu_layer), true);
  layer_add_child(window_layer, menu_layer_get_layer(screen->menu_layer));
#endif

  layer_add_child(window_layer, screen->spinner_layer);
  layer_set_hidden(screen->spinner_layer, !screen->is_loading);

  LOG_DEBUG("Initializing service summary");
  screen->service_summary_layer = service_summary_init(bounds_with_status_bar_no_padding(screen->window));
  LOG_DEBUG("Adding service summary to window");
  layer_add_child(window_layer, screen->service_summary_layer);
  layer_set_hidden(screen->service_summary_layer, !screen->load_complete);

  layer_add_child(window_layer, status_bar_layer_get_layer(screen->status_bar));
  LOG_DEBUG("Service screen initialized");

  if (screen->load_complete) {
    service_load_complete(screen);
  }
}

void service_window_disappear(Window *window) {
  ServiceScreen *screen = window_get_user_data(window);

  LOG_DEBUG("Service screen disappearing");

  if (screen->status_bar != NULL) {
    custom_status_bar_layer_destroy(screen->status_bar);
  }

  if (screen->spinner_layer != NULL) {
    spinner_layer_deinit(screen->spinner_layer);
  }

  if (screen->service_summary_layer != NULL) {
    service_summary_deinit(screen->service_summary_layer);
  }

  if (screen->error_layer != NULL) {
    text_layer_destroy(screen->error_layer);
  }

  free_menu_anim(screen);

#ifdef PBL_ROUND
  round_route_path_deinit(screen->route_path_layer);
#else
  menu_layer_destroy(screen->menu_layer);
#endif

  LOG_DEBUG("Service screen disappeared");
}

void service_window_unload(Window *window) {
  ServiceScreen *screen = window_get_user_data(window);

  service_screen_destroy(screen);
}

ServiceScreen *service_screen_create(char *service_id, char *origin) {
  ServiceScreen *screen = wm_alloc(sizeof(ServiceScreen));

  COPY_STRING(screen->service_info.serviceID, service_id);
  COPY_STRING(screen->service_info.origin, origin);

  LOG_DEBUG("Service screen initialized with service ID: %s and origin: %s", screen->service_info.serviceID, screen->service_info.origin);

  screen->window = window_create();

  screen->status_bar = NULL;
  screen->spinner_layer = NULL;
  screen->error_layer = NULL;
  screen->service_summary_layer = NULL;
  screen->action_menu = NULL;
  screen->root_level = NULL;
  screen->menu_prop_anim = NULL;
  screen->menu_anim = NULL;

  screen->load_complete = false;
  screen->is_loading = false;
  screen->is_menu_active = false;

  window_set_window_handlers(screen->window, (WindowHandlers){
                                                 .appear = service_window_appear,
                                                 .disappear = service_window_disappear,
                                                 .unload = service_window_unload,
                                             });

  window_set_user_data(screen->window, screen);

  LOG_DEBUG("Service screen created");

  prv_init_service_action_menu(screen);
  prv_load_service(screen);

  return screen;
}

void service_screen_push(ServiceScreen *screen) { window_manager_push_window(screen->window); }

void service_screen_destroy(ServiceScreen *screen) {
  window_destroy(screen->window);
  free(screen);
  LOG_DEBUG("Service screen destroyed");
}

ServiceScreen *top_window_as_service_screen() {
  Window *window = window_stack_get_top_window();
  return window_get_user_data(window);
}