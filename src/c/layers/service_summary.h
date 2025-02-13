#pragma once

#include <pebble.h>

typedef Layer ServiceSummaryLayer;

ServiceSummaryLayer *service_summary_init(GRect bounds);
void service_summary_deinit(ServiceSummaryLayer *layer);
void service_summary_set_destination(ServiceSummaryLayer *layer, char *destination);