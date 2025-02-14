#include "service_summary.h"
#include "../tocs.h"

typedef struct
{
  GRect bounds;
  char *destination;
  char *operator_code;
  char *time;
  OperatorInfo operator_info;
} ServiceSummaryData;

static GDrawCommandImage *s_train_icon;

static void service_summary_update_proc(Layer *layer, GContext *ctx)
{
  ServiceSummaryData *service_summary_data = (ServiceSummaryData *)layer_get_data(layer);
  GRect bounds = service_summary_data->bounds;
  GFont font = fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD);
  GFont operator_font = fonts_get_system_font(FONT_KEY_GOTHIC_14);
  GFont number_font = fonts_get_system_font(FONT_KEY_LECO_26_BOLD_NUMBERS_AM_PM);
  GColor bg_color = service_summary_data->operator_info.color;
  GColor fg_color = gcolor_legible_over(bg_color);

  graphics_context_set_fill_color(ctx, bg_color);
  graphics_context_set_text_color(ctx, fg_color);
  graphics_context_set_stroke_color(ctx, GColorRed);

  // Background
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);

  // Train icon
  gdraw_command_image_draw(ctx, s_train_icon, GPoint(5, 15));

  // Time
  int16_t icon_offset = 5 + 50 + 5;
  int16_t text_height = 30;
  GRect time_bounds = GRect(icon_offset, 15 + (50 - text_height) / 2, (bounds.size.w) - (icon_offset + 5), text_height);
  // graphics_draw_rect(ctx, time_bounds);
  graphics_draw_text(ctx, service_summary_data->time, number_font, time_bounds, GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);

  // Operator
  GRect operator_bounds = GRect(5, 15 + 50 + 5, (bounds.size.w) - 5, 20);
  graphics_draw_text(ctx, service_summary_data->operator_info.name, operator_font, operator_bounds, GTextOverflowModeWordWrap, GTextAlignmentLeft, NULL);

  // Destination
  GRect destination_bounds = GRect(5, 15 + 50 + 5 + 15, (bounds.size.w) - 5, 40);
  graphics_draw_text(ctx, service_summary_data->destination, font, destination_bounds, GTextOverflowModeWordWrap, GTextAlignmentLeft, NULL);
}

ServiceSummaryLayer *service_summary_init(GRect bounds)
{
  s_train_icon = gdraw_command_image_create_with_resource(RESOURCE_ID_TRAIN_SMALL);

  ServiceSummaryLayer *service_summary_layer = layer_create_with_data(bounds, sizeof(ServiceSummaryData));
  layer_set_update_proc(service_summary_layer, service_summary_update_proc);

  ServiceSummaryData *service_summary_data = (ServiceSummaryData *)layer_get_data(service_summary_layer);
  service_summary_data->bounds = bounds;
  service_summary_data->destination = "";
  service_summary_data->operator_code = "";
  service_summary_data->operator_info = (OperatorInfo){"Unknown", GColorBlack};
  service_summary_data->time = "23:59";

  return service_summary_layer;
}

void service_summary_deinit(ServiceSummaryLayer *layer)
{
  layer_destroy(layer);
  gdraw_command_image_destroy(s_train_icon);
}

void service_summary_set_data(ServiceSummaryLayer *layer, char *destination, char *operator_code, char *time)
{
  ServiceSummaryData *service_summary_data = (ServiceSummaryData *)layer_get_data(layer);
  service_summary_data->destination = destination;
  service_summary_data->operator_code = operator_code;
  service_summary_data->operator_info = operator_info(operator_code);
  service_summary_data->time = time;

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Got operator info %s = %s", operator_code, service_summary_data->operator_info.name);

  layer_mark_dirty(layer);
}

GColor service_summary_get_color(ServiceSummaryLayer *layer)
{
  ServiceSummaryData *service_summary_data = (ServiceSummaryData *)layer_get_data(layer);
  return service_summary_data->operator_info.color;
}
