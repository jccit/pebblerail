#include "journey_item.h"

const char *departed_text = "Departed";
const char *arrived_text = "Arrived";
const char *skipped_text = "Skipped";
const char *not_arrived_text = "Arriving";
const char *departing_text = "Departing";

#ifndef PBL_ROUND
const int line_padding = 4;
const int line_width = 4;
const int circle_radius = 8;
const int circle_45deg_point = 5;  // approximation of 8sin(0.25π), position on the circumference at a 45 degree angle
const int circle_stroke_width = 2;

int draw_route_line(GContext *ctx, GRect bounds, bool is_highlighted, bool start, bool end, CallingPointState state) {
  int left_pad = line_padding + circle_radius / 2;
  GRect top_line_rect = GRect(left_pad, 0, line_width, bounds.size.h / 2);
  GRect bottom_line_rect = GRect(left_pad, bounds.size.h / 2, line_width, bounds.size.h / 2);
  GPoint circle_centre = GPoint(left_pad + line_width / 2, bounds.size.h / 2);

  GColor fill_color = is_highlighted ? GColorWhite : GColorBlack;
  GColor accent_color = is_highlighted ? GColorBlack : GColorWhite;
#ifdef PBL_COLOR
  GColor warning_color = GColorRed;
  GColor warning_inset_color = GColorWhite;
#else
  GColor warning_color = fill_color;
  GColor warning_inset_color = accent_color;
#endif

  graphics_context_set_fill_color(ctx, fill_color);
  if (!start) {
    graphics_fill_rect(ctx, top_line_rect, 0, 0);
  }
  if (!end) {
    graphics_fill_rect(ctx, bottom_line_rect, 0, 0);
  }

  switch (state) {
    case CALLING_POINT_STATE_DEPARTED:
    case CALLING_POINT_STATE_ARRIVED:
      // solid circle
      graphics_fill_circle(ctx, circle_centre, circle_radius);
      break;

    case CALLING_POINT_STATE_SKIPPED:
      // red circle with cross
      graphics_context_set_fill_color(ctx, warning_color);
      graphics_fill_circle(ctx, circle_centre, circle_radius);
      graphics_context_set_fill_color(ctx, warning_inset_color);
      graphics_fill_circle(ctx, circle_centre, circle_radius - circle_stroke_width);

      GPoint cross_start = GPoint(circle_centre.x - circle_45deg_point, circle_centre.y + circle_45deg_point);
      GPoint cross_end = GPoint(circle_centre.x + circle_45deg_point, circle_centre.y - circle_45deg_point);

      graphics_context_set_stroke_color(ctx, warning_color);
      graphics_context_set_stroke_width(ctx, circle_stroke_width);
      graphics_draw_line(ctx, cross_start, cross_end);
      break;

    case CALLING_POINT_STATE_NOT_ARRIVED:
    default:
      // circle with outline
      graphics_fill_circle(ctx, circle_centre, circle_radius);
      graphics_context_set_fill_color(ctx, accent_color);
      graphics_fill_circle(ctx, circle_centre, circle_radius - circle_stroke_width);
      break;
  }

  return circle_radius + line_padding + line_width;
}
#endif

void journey_item_draw(GContext *ctx, const Layer *cell_layer, bool start, bool end, CallingPointEntry *callingPoint) {
  char combined_text[32];
  const char *state_text = departed_text;

  switch (callingPoint->state) {
    case CALLING_POINT_STATE_ARRIVED:
      state_text = arrived_text;
      break;
    case CALLING_POINT_STATE_SKIPPED:
      state_text = skipped_text;
      break;
    case CALLING_POINT_STATE_DEPARTED:
      state_text = departed_text;
      break;
    default:
      if (start) {
        state_text = departing_text;
      } else {
        state_text = not_arrived_text;
      }
      break;
  }

  GRect bounds = layer_get_bounds(cell_layer);
  snprintf(combined_text, sizeof(combined_text), "%s - %s", state_text, callingPoint->time);

#ifndef PBL_ROUND
  GTextAlignment alignment = GTextAlignmentLeft;
  bool is_highlighted = menu_cell_layer_is_highlighted(cell_layer);
  int text_start = draw_route_line(ctx, bounds, is_highlighted, start, end, callingPoint->state) + 8;
#else
  int text_start = 0;
  GTextAlignment alignment = GTextAlignmentCenter;
#endif

  GFont station_font = fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD);
  GRect station_box = GRect(text_start, 0, bounds.size.w - text_start, 20);
  graphics_draw_text(ctx, callingPoint->destination, station_font, station_box, GTextOverflowModeTrailingEllipsis, alignment, NULL);

  GFont time_font = fonts_get_system_font(FONT_KEY_LECO_20_BOLD_NUMBERS);
  GRect time_box = GRect(text_start, 18, bounds.size.w - text_start, 22);
  graphics_draw_text(ctx, callingPoint->time, time_font, time_box, GTextOverflowModeTrailingEllipsis, alignment, NULL);
}
