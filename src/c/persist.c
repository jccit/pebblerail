#include "persist.h"

#include "data.h"
#include "utils.h"

#define PERSIST_STATION_COUNT_KEY 1
#define PERSIST_STATION_KEY 2
#define PERSIST_MAX_STATION_COUNT 4

const size_t STATION_BUFFER_SIZE = sizeof(SavedStation) * PERSIST_MAX_STATION_COUNT;

SavedStation *persist_get_stations() {
  SavedStation *station_buffer = (SavedStation *)malloc(STATION_BUFFER_SIZE);

  persist_read_data(PERSIST_STATION_KEY, station_buffer, STATION_BUFFER_SIZE);

  return station_buffer;
}

uint32_t persist_get_station_count() { return persist_read_int(PERSIST_STATION_COUNT_KEY); }

void persist_save_station(Station *station) {
  SavedStation *station_buffer = persist_get_stations();
  uint32_t station_count = persist_get_station_count();

  if (station_count < PERSIST_MAX_STATION_COUNT) {
    COPY_STRING(station_buffer[station_count].name, station->name);
    COPY_STRING(station_buffer[station_count].crs, station->crs);
  } else {
    LOG_WARN("Persist: Max station count reached");
    return;
  }

  persist_write_int(PERSIST_STATION_COUNT_KEY, station_count + 1);
  persist_write_data(PERSIST_STATION_KEY, station_buffer, STATION_BUFFER_SIZE);

  free(station_buffer);

  LOG_DEBUG("Persist: Saved station '%s'. Total stations: %d", station->name, station_count + 1);
}

void persist_delete_station(char *crs) {
  SavedStation *station_buffer = persist_get_stations();
  uint32_t station_count = persist_get_station_count();

  for (uint32_t i = 0; i < station_count; i++) {
    if (strcmp(station_buffer[i].crs, crs) == 0) {
      for (uint32_t j = i; j < station_count - 1; j++) {
        station_buffer[j] = station_buffer[j + 1];
      }
      break;
    }
  }

  persist_write_int(PERSIST_STATION_COUNT_KEY, station_count - 1);
  persist_write_data(PERSIST_STATION_KEY, station_buffer, STATION_BUFFER_SIZE);

  free(station_buffer);

  LOG_DEBUG("Persist: Deleted station '%s'. Total stations: %d", crs, station_count - 1);
}