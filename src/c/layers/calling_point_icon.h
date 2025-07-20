#pragma once

#include <pebble.h>

#include "../data.h"

void calling_point_icon_draw(GContext *ctx, GPoint circle_centre, int circle_radius, GColor fill_color, GColor accent_color, GColor warning_color,
                             GColor warning_inset_color, CallingPointState state);