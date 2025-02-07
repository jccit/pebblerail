#include "journey_item.h"

void journey_item_draw(GContext *ctx, const Layer *cell_layer, bool start, CallingPointEntry *callingPoint)
{
  GRect bounds = layer_get_bounds(cell_layer);

  // char combined_text[32];
  // char *platform_display = s_calling_points[index].platform;

  // if (strcmp(platform_display, "-1") == 0)
  // {
  //   platform_display = "unknown";
  // }

  // snprintf(combined_text, sizeof(combined_text), "%s - Platform %s",
  //          s_calling_points[index].departureTime,
  //          platform_display);

  menu_cell_basic_draw(ctx, cell_layer, callingPoint->destination, callingPoint->departureTime, NULL);

  // TODO: Replace with bespoke ui
}
