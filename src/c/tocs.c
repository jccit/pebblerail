#include "tocs.h"

#define OPERATOR_INFO_SIZE 33

typedef struct {
  const char *key;
  int value;
} OperatorMap;

static const OperatorMap operator_info_map[] = {
    {"AW", 0},  {"CC", 1},  {"CH", 2},  {"CS", 3},  {"EM", 4},  {"ES", 5},  {"GC", 6},  {"GN", 7},  {"GR", 8},  {"GW", 9},  {"GX", 10},
    {"HC", 11}, {"HT", 12}, {"HX", 13}, {"IL", 14}, {"LD", 15}, {"LE", 16}, {"LM", 17}, {"LO", 18}, {"LT", 19}, {"ME", 20}, {"NT", 21},
    {"NY", 22}, {"PC", 23}, {"RT", 24}, {"SE", 25}, {"SJ", 26}, {"SN", 27}, {"SP", 28}, {"SR", 29}, {"SW", 30}, {"TL", 31}, {"TP", 32},
    {"TW", 33}, {"VT", 34}, {"WR", 35}, {"XC", 36}, {"XR", 37}, {"ZB", 38}, {"ZF", 39}, {"ZM", 40}, {NULL, 0}};

static int operator_info_index(const char *search_key) {
  for (const OperatorMap *entry = operator_info_map; entry->key != NULL; entry++) {
    if (strcmp(entry->key, search_key) == 0) {
      return entry->value;
    }
  }

  return -1;
}

OperatorInfo operator_info(char *requested_code) {
  ResHandle handle = resource_get_handle(RESOURCE_ID_OPERATOR_INFO);
  size_t res_size = resource_size(handle);

  int index = operator_info_index(requested_code);
  if (index == -1) {
    return (OperatorInfo){"Unknown", GColorWhite};
  }

  uint8_t *op_buffer = malloc(OPERATOR_INFO_SIZE);
  size_t loaded_bytes = resource_load_byte_range(handle, index * OPERATOR_INFO_SIZE, op_buffer, OPERATOR_INFO_SIZE);

  if (loaded_bytes != OPERATOR_INFO_SIZE) {
    return (OperatorInfo){"Unknown", GColorWhite};
  }

  char *name = (char *)op_buffer;

#ifdef PBL_BW
  GColor colour = GColorWhite;
#else
  GColor colour = (GColor8){.argb = op_buffer[32]};
#endif

  return (OperatorInfo){name, colour};
}
