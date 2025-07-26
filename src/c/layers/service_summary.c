#include "service_summary.h"

#include "../tocs.h"
#include "../utils.h"
#include "../window_manager.h"

const int SUMMARY_ANIMATION_DURATION = 150;
const int SUMMARY_ANIMATION_Y_OFFSET = STATUS_BAR_LAYER_HEIGHT;

typedef struct {
  GRect bounds;
  char *origin;
  char *destination;
  char *operator_code;
  char *time;
  char *lateness;
  char *platform;
  char *reason;
  OperatorInfo operator_info;
  CallingPointState state;

  PropertyAnimation *prop_anim;
  Animation *anim;
} ServiceSummaryData;

static GDrawCommandImage *s_train_icon = NULL;
static GBitmap *s_down_icon = NULL;
static GRect s_down_icon_bounds;
static GFont s_destination_font;
static GFont s_origin_font;
static GFont s_operator_font;
static GFont s_number_font;

static void service_summary_load_icons(ServiceSummaryData *service_summary_data) {
  if (s_train_icon == NULL) {
    s_train_icon = gdraw_command_image_create_with_resource(RESOURCE_ID_TRAIN_SMALL);
    LOG_DEBUG_VERBOSE("train icon = %p", s_train_icon);
  }

#ifndef PBL_PLATFORM_APLITE
  if (s_down_icon == NULL) {
    bool use_white_arrow = gcolor_equal(gcolor_legible_over(service_summary_data->operator_info.color), GColorWhite);
    if (s_down_icon != NULL) {
      gbitmap_destroy(s_down_icon);
      s_down_icon = NULL;
    }
    s_down_icon = gbitmap_create_with_resource(use_white_arrow ? RESOURCE_ID_DOWN_ARROW_WHITE : RESOURCE_ID_DOWN_ARROW_BLACK);
  }
#endif
}

static void service_summary_update_proc(Layer *layer, GContext *ctx) {
  ServiceSummaryData *service_summary_data = (ServiceSummaryData *)layer_get_data(layer);
  GRect bounds = service_summary_data->bounds;
  GColor bg_color = service_summary_data->operator_info.color;
  GColor fg_color = gcolor_legible_over(bg_color);
  bool show_reason = service_summary_data->reason != NULL;
  bool show_lateness = service_summary_data->lateness != NULL;
  bool show_platform = service_summary_data->platform != NULL;

  service_summary_load_icons(service_summary_data);

  graphics_context_set_fill_color(ctx, bg_color);
  graphics_context_set_text_color(ctx, fg_color);
  graphics_context_set_stroke_color(ctx, GColorRed);
  graphics_context_set_compositing_mode(ctx, GCompOpSet);

  int16_t icon_size = 50;
  int16_t horizontal_margin = 5 * 2;
  int16_t top_margin = 10;

#ifdef PBL_ROUND
  int16_t icon_margin = 25;
  int16_t icon_offset = icon_margin + icon_size + 5;
  GTextAlignment time_alignment = GTextAlignmentLeft;
  GTextAlignment status_alignment = GTextAlignmentCenter;
#else
  int16_t icon_margin = 5;
  int16_t icon_offset = icon_margin + icon_size + 5;
  GTextAlignment time_alignment = GTextAlignmentCenter;
  GTextAlignment status_alignment = GTextAlignmentLeft;
#endif

  // Background
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);

  // Train icon
  gdraw_command_image_draw(ctx, s_train_icon, GPoint(icon_margin, top_margin));

  // Time
  int16_t text_height = 30;
  int16_t time_vertical_pos = (icon_size - text_height) / 2;
  int16_t time_width = (bounds.size.w) - (icon_offset + horizontal_margin);
  GRect time_bounds = GRect(icon_offset, top_margin + time_vertical_pos, time_width, text_height);
  graphics_draw_text(ctx, service_summary_data->time, s_number_font, time_bounds, GTextOverflowModeWordWrap, time_alignment, NULL);

  // Platform
  if (show_platform) {
    GRect platform_bounds = GRect(icon_offset, time_vertical_pos - 1, time_width, 20);
    graphics_draw_text(ctx, service_summary_data->platform, s_operator_font, platform_bounds, GTextOverflowModeWordWrap, time_alignment, NULL);
  }

  // Late
  if (show_lateness) {
    GRect late_bounds = GRect(icon_offset, time_vertical_pos + 34, time_width, 20);
    graphics_draw_text(ctx, service_summary_data->lateness, s_operator_font, late_bounds, GTextOverflowModeWordWrap, time_alignment, NULL);
  }

  int16_t vertical_cursor = top_margin + icon_size + 2;

  // Operator / reason
  char *operator_text = show_reason ? service_summary_data->reason : service_summary_data->operator_info.name;
  GRect operator_bounds = GRect(5, vertical_cursor, (bounds.size.w) - 10, 40);
  GSize operator_size =
      graphics_draw_text_get_size(ctx, operator_text, s_operator_font, operator_bounds, GTextOverflowModeWordWrap, status_alignment);
  vertical_cursor += operator_size.h + 2;

  // Origin
  char origin_text[64];
  snprintf(origin_text, sizeof(origin_text), "From: %s", service_summary_data->origin);
  GRect origin_bounds = GRect(horizontal_margin / 2, vertical_cursor, (bounds.size.w) - horizontal_margin, 40);
  GSize origin_size = graphics_draw_text_get_size(ctx, origin_text, s_origin_font, origin_bounds, GTextOverflowModeWordWrap, status_alignment);
  vertical_cursor += origin_size.h + 1;

  // Destination
  char destination_text[64];
  snprintf(destination_text, sizeof(destination_text), "To: %s", service_summary_data->destination);
  GRect destination_bounds = GRect(horizontal_margin / 2, vertical_cursor, (bounds.size.w) - horizontal_margin, 40);
  // GSize destination_size = graphics_draw_text_get_size(ctx, destination_text, s_destination_font, destination_bounds, GTextOverflowModeWordWrap,
  // status_alignment);

  // Down arrow
  if (s_down_icon != NULL) {
    GPoint icon_position = GPoint((bounds.size.w / 2) - (s_down_icon_bounds.size.w / 2), bounds.size.h - s_down_icon_bounds.size.h - 4);
    GRect icon_bounds = GRect(icon_position.x, icon_position.y, s_down_icon_bounds.size.w, s_down_icon_bounds.size.h);
    graphics_draw_bitmap_in_rect(ctx, s_down_icon, icon_bounds);
  }
}

ServiceSummaryLayer *service_summary_init(GRect bounds) {
  s_train_icon = gdraw_command_image_create_with_resource(RESOURCE_ID_TRAIN_SMALL);
  LOG_DEBUG_VERBOSE("train icon = %p", s_train_icon);

  s_destination_font = fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD);
  s_origin_font = fonts_get_system_font(FONT_KEY_GOTHIC_18);
  s_operator_font = fonts_get_system_font(FONT_KEY_GOTHIC_14);
  s_number_font = fonts_get_system_font(FONT_KEY_LECO_26_BOLD_NUMBERS_AM_PM);

  GBitmap *test_icon = gbitmap_create_with_resource(RESOURCE_ID_DOWN_ARROW_BLACK);
  s_down_icon_bounds = gbitmap_get_bounds(test_icon);
  gbitmap_destroy(test_icon);

  ServiceSummaryLayer *service_summary_layer = layer_create_with_data(bounds, sizeof(ServiceSummaryData));
  layer_set_update_proc(service_summary_layer, service_summary_update_proc);

  ServiceSummaryData *service_summary_data = (ServiceSummaryData *)layer_get_data(service_summary_layer);
  service_summary_data->bounds = bounds;
  service_summary_data->destination = "";
  service_summary_data->operator_code = "";
  service_summary_data->operator_info = (OperatorInfo){NULL, GColorBlack};
  service_summary_data->time = "23:59";

  service_summary_data->prop_anim = NULL;
  service_summary_data->anim = NULL;

  return service_summary_layer;
}

void service_summary_free_anim(ServiceSummaryLayer *layer) {
  ServiceSummaryData *service_summary_data = (ServiceSummaryData *)layer_get_data(layer);
  if (service_summary_data->prop_anim != NULL) {
    property_animation_destroy(service_summary_data->prop_anim);
  }
  if (service_summary_data->anim != NULL) {
    animation_destroy(service_summary_data->anim);
  }
  service_summary_data->prop_anim = NULL;
  service_summary_data->anim = NULL;
}

void service_summary_deinit(ServiceSummaryLayer *layer) {
  ServiceSummaryData *service_summary_data = (ServiceSummaryData *)layer_get_data(layer);
  if (service_summary_data->lateness != NULL) {
    free(service_summary_data->lateness);
  }
  if (service_summary_data->platform != NULL) {
    free(service_summary_data->platform);
  }
  if (service_summary_data->reason != NULL) {
    free(service_summary_data->reason);
  }
  if (service_summary_data->operator_info.name != NULL) {
    free(service_summary_data->operator_info.name);
  }

  service_summary_free_anim(layer);

  gdraw_command_image_destroy(s_train_icon);
  gbitmap_destroy(s_down_icon);
  s_train_icon = NULL;
  s_down_icon = NULL;

  layer_destroy(layer);
}

void service_summary_set_data(ServiceSummaryLayer *layer, char *origin, char *destination, char *operator_code, char *time, char *reason,
                              char *platform, CallingPointState state) {
  ServiceSummaryData *service_summary_data = (ServiceSummaryData *)layer_get_data(layer);
  service_summary_data->origin = origin;
  service_summary_data->destination = destination;
  service_summary_data->operator_code = operator_code;
  service_summary_data->reason = reason;
  service_summary_data->state = state;

  if (service_summary_data->operator_info.name != NULL) {
    free(service_summary_data->operator_info.name);
  }

  service_summary_data->operator_info = operator_info(operator_code);

  char *space_ptr = strchr(time, ' ');
  if (space_ptr == NULL) {
    service_summary_data->time = time;
    service_summary_data->lateness = NULL;
  } else {
    size_t time_length = space_ptr - time;
    strncpy(service_summary_data->time, time, time_length);
    service_summary_data->time[time_length] = '\0';

    char *lateness_suffix = " late";
    char *lateness_start = strchr(time, '(') + 1;
    char *lateness_end = strchr(time, 'm') + 1;
    size_t lateness_length = lateness_end - lateness_start;
    size_t lateness_total_length = lateness_length + strlen(lateness_suffix);

    char *lateness = wm_alloc(lateness_total_length + 1);
    strncpy(lateness, lateness_start, lateness_length);
    strcat(lateness, lateness_suffix);
    lateness[lateness_total_length] = '\0';
    service_summary_data->lateness = lateness;
  }

  char *platform_prefix = "Plat: ";
  uint16_t platform_length = strlen(platform_prefix) + strlen(platform);
  char *platform_text = wm_alloc(platform_length + 1);
  strcpy(platform_text, platform_prefix);
  strcat(platform_text, platform);
  service_summary_data->platform = platform_text;

  service_summary_load_icons(service_summary_data);

  layer_mark_dirty(layer);
}

GColor service_summary_get_color(ServiceSummaryLayer *layer) {
  ServiceSummaryData *service_summary_data = (ServiceSummaryData *)layer_get_data(layer);
  return service_summary_data->operator_info.color;
}

void service_summary_animate_out(ServiceSummaryLayer *summary_layer) {
  Layer *layer = (Layer *)summary_layer;
  ServiceSummaryData *service_summary_data = (ServiceSummaryData *)layer_get_data(layer);

  GRect start = service_summary_data->bounds;
  GRect end = GRect(start.origin.x, start.origin.y - (start.size.h + SUMMARY_ANIMATION_Y_OFFSET), start.size.w, start.size.h);

  LOG_DEBUG_VERBOSE("Service summary anim start: %d, %d, %d, %d", start.origin.x, start.origin.y, start.size.w, start.size.h);
  LOG_DEBUG_VERBOSE("Service summary anim end: %d, %d, %d, %d", end.origin.x, end.origin.y, end.size.w, end.size.h);

  service_summary_free_anim(summary_layer);

  service_summary_data->prop_anim = property_animation_create_layer_frame(layer, &start, &end);
  service_summary_data->anim = property_animation_get_animation(service_summary_data->prop_anim);
  animation_set_curve(service_summary_data->anim, AnimationCurveEaseOut);
  animation_set_duration(service_summary_data->anim, SUMMARY_ANIMATION_DURATION);
  animation_schedule(service_summary_data->anim);
}

void service_summary_animate_in(ServiceSummaryLayer *summary_layer) {
  Layer *layer = (Layer *)summary_layer;
  ServiceSummaryData *service_summary_data = (ServiceSummaryData *)layer_get_data(layer);

  GRect end = service_summary_data->bounds;
  GRect start = GRect(end.origin.x, end.origin.y - (end.size.h + SUMMARY_ANIMATION_Y_OFFSET), end.size.w, end.size.h);

  LOG_DEBUG_VERBOSE("Service summary anim start: %d, %d, %d, %d", start.origin.x, start.origin.y, start.size.w, start.size.h);
  LOG_DEBUG_VERBOSE("Service summary anim end: %d, %d, %d, %d", end.origin.x, end.origin.y, end.size.w, end.size.h);

  service_summary_free_anim(summary_layer);

  service_summary_data->prop_anim = property_animation_create_layer_frame(layer, &start, &end);
  service_summary_data->anim = property_animation_get_animation(service_summary_data->prop_anim);
  animation_set_curve(service_summary_data->anim, AnimationCurveEaseOut);
  animation_set_duration(service_summary_data->anim, SUMMARY_ANIMATION_DURATION);
  animation_schedule(service_summary_data->anim);
}
