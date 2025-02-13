#include "service_summary.h"

typedef struct
{
  GRect bounds;
  char *destination;
  GColor bg_color;
} ServiceSummaryData;

static void service_summary_update_proc(Layer *layer, GContext *ctx)
{
  ServiceSummaryData *service_summary_data = (ServiceSummaryData *)layer_get_data(layer);
  GFont font = fonts_get_system_font(FONT_KEY_GOTHIC_18);
  GColor bg_color = service_summary_data->bg_color;
  GColor fg_color = gcolor_legible_over(bg_color);

  APP_LOG(APP_LOG_LEVEL_DEBUG, "bounds: %d, %d, %d, %d", service_summary_data->bounds.origin.x, service_summary_data->bounds.origin.y, service_summary_data->bounds.size.w, service_summary_data->bounds.size.h);

  // Background
  graphics_context_set_fill_color(ctx, bg_color);
  graphics_fill_rect(ctx, service_summary_data->bounds, 0, GCornerNone);

  // Destination
  graphics_context_set_text_color(ctx, fg_color);
  graphics_draw_text(ctx, service_summary_data->destination, font, service_summary_data->bounds, GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
}

ServiceSummaryLayer *service_summary_init(GRect bounds)
{
  ServiceSummaryLayer *service_summary_layer = layer_create_with_data(bounds, sizeof(ServiceSummaryData));
  layer_set_update_proc(service_summary_layer, service_summary_update_proc);

  ServiceSummaryData *service_summary_data = (ServiceSummaryData *)layer_get_data(service_summary_layer);
  service_summary_data->bounds = bounds;
  service_summary_data->destination = "Liverpool Central";
  service_summary_data->bg_color = GColorBlue;

  return service_summary_layer;
}

void service_summary_deinit(ServiceSummaryLayer *layer)
{
  layer_destroy(layer);
}

void service_summary_set_destination(ServiceSummaryLayer *layer, char *destination)
{
  ServiceSummaryData *service_summary_data = (ServiceSummaryData *)layer_get_data(layer);
  service_summary_data->destination = destination;
  layer_mark_dirty(layer);
}
