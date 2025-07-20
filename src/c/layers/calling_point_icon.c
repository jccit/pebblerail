#include "calling_point_icon.h"

const int circle_45deg_point = 5;  // approximation of 8sin(0.25π), position on the circumference at a 45 degree angle
const int circle_stroke_width = 3;

void calling_point_icon_draw(GContext *ctx, GPoint circle_centre, int circle_radius, GColor fill_color, GColor accent_color, GColor warning_color,
                             GColor warning_inset_color, CallingPointState state) {
  graphics_context_set_fill_color(ctx, fill_color);

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
}