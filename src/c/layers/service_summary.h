#pragma once

#include <pebble.h>
#include "../data.h"

typedef Layer ServiceSummaryLayer;

ServiceSummaryLayer *service_summary_init(GRect bounds);
void service_summary_deinit(ServiceSummaryLayer *layer);
void service_summary_set_data(ServiceSummaryLayer *layer, char *origin, char *destination, char *operator_code, char *time, char *reason, char *platform, CallingPointState state);
GColor service_summary_get_color(ServiceSummaryLayer *layer);