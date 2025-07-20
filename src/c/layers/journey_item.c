#include "journey_item.h"

char *departed_text = "Departed";
char *arrived_text = "Arrived";
char *skipped_text = "Skipped";
char *not_arrived_text = "Arriving";
char *departing_text = "Departing";

void journey_item_draw(GContext *ctx, const Layer *cell_layer, bool start, CallingPointEntry *callingPoint) {
  // GRect bounds = layer_get_bounds(cell_layer);

  char combined_text[32];
  char *state_text = departed_text;

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

  snprintf(combined_text, sizeof(combined_text), "%s - %s", state_text, callingPoint->time);

  menu_cell_basic_draw(ctx, cell_layer, callingPoint->destination, combined_text, NULL);

  // TODO: Replace with bespoke ui
}
