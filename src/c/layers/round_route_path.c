#include "round_route_path.h"

#include "../tocs.h"
#include "../utils.h"
#include "calling_point_icon.h"

#ifdef PBL_ROUND
typedef struct {
  CallingPointEntry *callingPoints;
  int callingPointCount;
  int selected;
  GColor operator_color;
  GColor contrast_color;
  TextLayer *destination_layer;
  TextLayer *time_layer;
} RoundRoutePathData;

typedef enum {
  ScrollDirectionDown,
  ScrollDirectionUp,
} ScrollDirection;

static const uint32_t SCROLL_DURATION = 100 * 2;
static const int16_t SCROLL_DIST_OUT = 20;
static const int16_t SCROLL_DIST_IN = 8;
static const int16_t BG_RADIUS = 62;

Animation *round_route_out_anim(Layer *layer, uint32_t duration, int16_t dy) {
  GPoint to_origin = GPoint(0, dy);
  Animation *result = (Animation *)property_animation_create_bounds_origin(layer, NULL, &to_origin);
  animation_set_duration(result, duration);
  animation_set_curve(result, AnimationCurveLinear);
  return result;
}

Animation *round_route_in_anim(Layer *layer, uint32_t duration, int16_t dy) {
  GPoint from_origin = GPoint(0, dy);
  Animation *result = (Animation *)property_animation_create_bounds_origin(layer, &from_origin, &GPointZero);
  animation_set_duration(result, duration);
  animation_set_curve(result, AnimationCurveEaseOut);
  return result;
}

void update_text_after_anim(Animation *animation, bool finished, void *context) {
  RoundRoutePathData *data = (RoundRoutePathData *)context;
  CallingPointEntry *callingPoint = &data->callingPoints[data->selected];

  text_layer_set_text(data->destination_layer, callingPoint->destination);
  text_layer_set_text(data->time_layer, callingPoint->time);
}

Animation *round_route_text_anim(Layer *layer, ScrollDirection direction) {
  RoundRoutePathData *data = (RoundRoutePathData *)layer_get_data(layer);
  Layer *destination_layer = text_layer_get_layer(data->destination_layer);
  Layer *time_layer = text_layer_get_layer(data->time_layer);

  const int16_t in_dy = (direction == ScrollDirectionDown) ? -SCROLL_DIST_IN : SCROLL_DIST_IN;
  const int16_t out_dy = (direction == ScrollDirectionDown) ? -SCROLL_DIST_OUT : SCROLL_DIST_OUT;

  Animation *out_destination = round_route_out_anim(destination_layer, SCROLL_DURATION, out_dy);
  Animation *in_destination = round_route_in_anim(destination_layer, SCROLL_DURATION, in_dy);
  Animation *out_time = round_route_out_anim(time_layer, SCROLL_DURATION, out_dy);
  Animation *in_time = round_route_in_anim(time_layer, SCROLL_DURATION, in_dy);

  animation_set_handlers(out_destination, (AnimationHandlers){.stopped = update_text_after_anim}, data);

  return animation_spawn_create(animation_sequence_create(out_destination, in_destination, NULL), animation_sequence_create(out_time, in_time, NULL),
                                NULL);
}

void round_route_path_draw(GContext *ctx, GRect bounds, CallingPointEntry *callingPoints, int callingPointCount, int selected) {
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
    bool is_selected = i == selected;
    GColor accent_color = is_selected ? GColorGreen : GColorWhite;

    calling_point_icon_draw(ctx, circle_point, circle_radius, fill_color, accent_color, warning_color, warning_inset_color, callingPoints[i].state);
  }
}

void round_route_path_update_proc(Layer *layer, GContext *ctx) {
  RoundRoutePathData *data = (RoundRoutePathData *)layer_get_data(layer);

  if (data->callingPoints == NULL || data->callingPointCount == 0) {
    return;
  }

  GRect bounds = layer_get_bounds(layer);
  GPoint center = GPoint(bounds.size.w / 2, bounds.size.h / 2);
  graphics_context_set_fill_color(ctx, data->operator_color);
  graphics_fill_circle(ctx, center, BG_RADIUS);

  round_route_path_draw(ctx, bounds, data->callingPoints, data->callingPointCount, data->selected);
}

Layer *round_route_path_init(GRect bounds) {
  Layer *layer = layer_create_with_data(bounds, sizeof(RoundRoutePathData));
  RoundRoutePathData *data = (RoundRoutePathData *)layer_get_data(layer);
  data->callingPoints = NULL;
  data->callingPointCount = 0;

  int text_centre = bounds.size.w / 2;
  int station_height = 20;
  int time_height = 22;
  int text_top = text_centre - (station_height + time_height) / 2;
  int text_margin = BG_RADIUS / 2 + 5;
  int text_width = bounds.size.w - text_margin * 2;

  GFont station_font = fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD);
  GFont time_font = fonts_get_system_font(FONT_KEY_LECO_20_BOLD_NUMBERS);
  GRect station_box = GRect(text_margin, text_top, text_width, station_height);
  GRect time_box = GRect(text_margin, text_top + station_height, text_width, time_height);

  data->destination_layer = text_layer_create(station_box);
  data->time_layer = text_layer_create(time_box);

  text_layer_set_font(data->destination_layer, station_font);
  text_layer_set_font(data->time_layer, time_font);
  text_layer_set_text_alignment(data->destination_layer, GTextAlignmentCenter);
  text_layer_set_text_alignment(data->time_layer, GTextAlignmentCenter);

  layer_add_child(layer, text_layer_get_layer(data->destination_layer));
  layer_add_child(layer, text_layer_get_layer(data->time_layer));

  layer_set_update_proc(layer, round_route_path_update_proc);
  return layer;
}

void round_route_path_set_data(Layer *layer, CallingPointEntry *callingPoints, int callingPointCount, char *operatorCode) {
  RoundRoutePathData *data = (RoundRoutePathData *)layer_get_data(layer);
  data->callingPoints = callingPoints;
  data->callingPointCount = callingPointCount;
  data->selected = 0;

  OperatorInfo op_info = operator_info(operatorCode);
  data->operator_color = op_info.color;
  data->contrast_color = gcolor_legible_over(data->operator_color);

  text_layer_set_text_color(data->destination_layer, data->contrast_color);
  text_layer_set_text_color(data->time_layer, data->contrast_color);
  text_layer_set_background_color(data->destination_layer, data->operator_color);
  text_layer_set_background_color(data->time_layer, data->operator_color);

  text_layer_set_text(data->destination_layer, callingPoints[0].destination);
  text_layer_set_text(data->time_layer, callingPoints[0].time);
}

void round_route_set_selected(Layer *layer, int selected) {
  RoundRoutePathData *data = (RoundRoutePathData *)layer_get_data(layer);
  ScrollDirection direction = (selected > data->selected) ? ScrollDirectionDown : ScrollDirectionUp;
  data->selected = selected;

  Animation *anim = round_route_text_anim(layer, direction);
  animation_schedule(anim);

  layer_mark_dirty(layer);
}

bool round_route_next_calling_point(Layer *layer) {
  RoundRoutePathData *data = (RoundRoutePathData *)layer_get_data(layer);
  if (data->selected >= data->callingPointCount - 1) {
    return true;
  }
  round_route_set_selected(layer, data->selected + 1);
  LOG_DEBUG_VERBOSE("Selected calling point: %d", data->selected);
  return false;
}

bool round_route_previous_calling_point(Layer *layer) {
  RoundRoutePathData *data = (RoundRoutePathData *)layer_get_data(layer);
  if (data->selected <= 0) {
    return true;
  }
  round_route_set_selected(layer, data->selected - 1);
  LOG_DEBUG_VERBOSE("Selected calling point: %d", data->selected);
  return false;
}

void round_route_path_deinit(Layer *layer) {
  RoundRoutePathData *data = (RoundRoutePathData *)layer_get_data(layer);
  text_layer_destroy(data->destination_layer);
  text_layer_destroy(data->time_layer);
  layer_destroy(layer);
}
#endif