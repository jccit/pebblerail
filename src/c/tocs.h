#pragma once

#include <pebble.h>

typedef struct {
  char *name;
  GColor color;
  bool metro;
} OperatorInfo;

OperatorInfo operator_info(char *code);