#include "round_route_path.h"

#include "calling_point_icon.h"

typedef struct {
  CallingPointEntry *callingPoints;
  int callingPointCount;
  int selected;
} RoundRoutePathData;

void round_route_path_draw(GContext *ctx, GRect bounds, CallingPointEntry *callingPoints, int callingPointCount) {
  const int circle_screen_padding = 15;
  const int circle_radius = 10;
  const int circle_arc_size = 180 + 90;
  const int circle_arc_offset = 180 + 45;
  const int round_line_width = 4;

  GColor fill_color = GColorBlack;
  GColor accent_color = GColorWhite;
#ifdef PBL_COLOR
  GColor warning_color = GColorRed;
  GColor warning_inset_color = GColorWhite;
#else
  GColor warning_color = fill_color;
  GColor warning_inset_color = accent_color;
#endif

  GRect circle_bounds = GRect(bounds.origin.x + circle_screen_padding, bounds.origin.y + circle_screen_padding,
                              bounds.size.w - circle_screen_padding * 2, bounds.size.h - circle_screen_padding * 2);

  int start_angle = DEG_TO_TRIGANGLE(circle_arc_offset);
  int end_angle = DEG_TO_TRIGANGLE(circle_arc_offset + circle_arc_size);

  graphics_context_set_stroke_color(ctx, fill_color);
  graphics_context_set_stroke_width(ctx, round_line_width);
  graphics_draw_arc(ctx, circle_bounds, GOvalScaleModeFitCircle, start_angle, end_angle);

  for (int i = 0; i < callingPointCount; i++) {
    int angle = start_angle + ((end_angle - start_angle) * i) / (callingPointCount - 1);
    GPoint circle_point = gpoint_from_polar(circle_bounds, GOvalScaleModeFitCircle, angle);
    calling_point_icon_draw(ctx, circle_point, circle_radius, fill_color, accent_color, warning_color, warning_inset_color, callingPoints[i].state);
  }
}

void round_route_calling_point_draw(GContext *ctx, GRect bounds, CallingPointEntry *callingPoints, int selected_index) {
  graphics_context_set_text_color(ctx, GColorBlack);

  GTextAlignment alignment = GTextAlignmentCenter;
  CallingPointEntry *callingPoint = &callingPoints[selected_index];

  int text_centre = bounds.size.w / 2;
  int station_height = 20;
  int time_height = 22;
  int text_top = text_centre - (station_height + time_height) / 2;
  int text_margin = 30;
  int text_width = bounds.size.w - text_margin * 2;

  GFont station_font = fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD);
  GRect station_box = GRect(text_margin, text_top, text_width, station_height);
  graphics_draw_text(ctx, callingPoint->destination, station_font, station_box, GTextOverflowModeTrailingEllipsis, alignment, NULL);

  GFont time_font = fonts_get_system_font(FONT_KEY_LECO_20_BOLD_NUMBERS);
  GRect time_box = GRect(text_margin, text_top + station_height, text_width, time_height);
  graphics_draw_text(ctx, callingPoint->time, time_font, time_box, GTextOverflowModeTrailingEllipsis, alignment, NULL);
}

void round_route_path_update_proc(Layer *layer, GContext *ctx) {
  RoundRoutePathData *data = (RoundRoutePathData *)layer_get_data(layer);

  if (data->callingPoints == NULL || data->callingPointCount == 0) {
    return;
  }

  GRect bounds = layer_get_bounds(layer);
  round_route_path_draw(ctx, bounds, data->callingPoints, data->callingPointCount);
  round_route_calling_point_draw(ctx, bounds, data->callingPoints, data->selected);
}

Layer *round_route_path_init(GRect bounds) {
  Layer *layer = layer_create_with_data(bounds, sizeof(RoundRoutePathData));
  RoundRoutePathData *data = (RoundRoutePathData *)layer_get_data(layer);
  data->callingPoints = NULL;
  data->callingPointCount = 0;

  layer_set_update_proc(layer, round_route_path_update_proc);
  return layer;
}

void round_route_path_set_data(Layer *layer, CallingPointEntry *callingPoints, int callingPointCount) {
  RoundRoutePathData *data = (RoundRoutePathData *)layer_get_data(layer);
  data->callingPoints = callingPoints;
  data->callingPointCount = callingPointCount;
}

void round_route_set_selected(Layer *layer, int selected) {
  RoundRoutePathData *data = (RoundRoutePathData *)layer_get_data(layer);
  data->selected = selected;
  layer_mark_dirty(layer);
}

bool round_route_next_calling_point(Layer *layer) {
  RoundRoutePathData *data = (RoundRoutePathData *)layer_get_data(layer);
  if (data->selected >= data->callingPointCount - 1) {
    return true;
  }
  round_route_set_selected(layer, data->selected + 1);
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Selected calling point: %d", data->selected);
  return false;
}

bool round_route_previous_calling_point(Layer *layer) {
  RoundRoutePathData *data = (RoundRoutePathData *)layer_get_data(layer);
  if (data->selected <= 0) {
    return true;
  }
  round_route_set_selected(layer, data->selected - 1);
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Selected calling point: %d", data->selected);
  return false;
}
