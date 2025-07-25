#include "spinner_layer.h"

#include "../utils.h"

const int spinner_interval_ms = 50;
const uint16_t spinner_radius = 25;
const uint32_t spinner_angle_gap = 60;
const uint32_t spinner_angle_increment = 10;
const uint32_t spinner_line_width = 8;

typedef struct {
  uint32_t angle;
  GPoint center;
  GRect bounds;
  AppTimer *timer;
} SpinnerData;

void spinner_layer_update_proc(Layer *layer, GContext *ctx) {
  SpinnerData *spinner_data = (SpinnerData *)layer_get_data(layer);

  spinner_data->angle += spinner_angle_increment;
  if (spinner_data->angle >= 360) {
    spinner_data->angle = 0;
  }

  int angle_zero = DEG_TO_TRIGANGLE(0);
  int angle_full = DEG_TO_TRIGANGLE(360);

  int angle_start = DEG_TO_TRIGANGLE(spinner_data->angle);
  int angle_end = DEG_TO_TRIGANGLE(spinner_data->angle + spinner_angle_gap);

  graphics_context_set_fill_color(ctx, GColorLightGray);
  graphics_fill_radial(ctx, spinner_data->bounds, GOvalScaleModeFitCircle, spinner_line_width, angle_zero, angle_full);

  // Draw the loading arc
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_radial(ctx, spinner_data->bounds, GOvalScaleModeFitCircle, spinner_line_width, angle_start, angle_end);

  GRect text_bounds = GRect(0, spinner_data->center.y + spinner_radius + 5, PBL_DISPLAY_WIDTH, 30);
  GFont font = fonts_get_system_font(FONT_KEY_GOTHIC_18);
  graphics_context_set_text_color(ctx, GColorBlack);
  graphics_draw_text(ctx, "Loading...", font, text_bounds, GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
}

void spinner_timer_callback(void *context) {
  Layer *spinner_layer = (Layer *)context;
  SpinnerData *spinner_data = (SpinnerData *)layer_get_data(spinner_layer);

  layer_mark_dirty(spinner_layer);

  spinner_data->timer = app_timer_register(spinner_interval_ms, spinner_timer_callback, spinner_layer);
}

SpinnerLayer *spinner_layer_init(GRect bounds) {
  SpinnerLayer *spinner_layer = layer_create_with_data(bounds, sizeof(SpinnerData));
  layer_set_update_proc(spinner_layer, spinner_layer_update_proc);
  layer_mark_dirty(spinner_layer);

  GPoint center = GPoint(bounds.size.w / 2, bounds.size.h / 2);

  SpinnerData *spinner_data = (SpinnerData *)layer_get_data(spinner_layer);
  spinner_data->angle = 0;
  spinner_data->center = center;
  spinner_data->bounds = GRect(center.x - spinner_radius, center.y - spinner_radius, spinner_radius * 2, spinner_radius * 2);
  spinner_data->timer = NULL;

  // Setup timer for the animation
  app_timer_register(spinner_interval_ms, spinner_timer_callback, spinner_layer);

  return spinner_layer;
}

void spinner_layer_deinit(Layer *spinner_layer) {
  if (spinner_layer == NULL) {
    LOG_WARN("Tried to deinit NULL spinner layer");
    return;
  }

  SpinnerData *spinner_data = (SpinnerData *)layer_get_data(spinner_layer);
  if (spinner_data != NULL && spinner_data->timer != NULL) {
    app_timer_cancel(spinner_data->timer);
  }

  layer_destroy(spinner_layer);
}
