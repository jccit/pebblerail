#pragma once

#include <pebble.h>

typedef struct
{
  char *name;
  GColor color;
} OperatorInfo;

OperatorInfo operator_info(char *code);