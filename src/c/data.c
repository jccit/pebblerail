#include "data.h"

const uint32_t inbox_size = 512;
const uint32_t outbox_size = 256;
const uint32_t CACHE_COMMAND_DELAY_MS = 500;

static bool js_ready = false;
static CommandType cached_command = COMMAND_TYPE_UNKNOWN;

static void (*s_closest_station_callback)(DictionaryIterator *iter) = NULL;
static void (*s_departures_callback)(DictionaryIterator *iter) = NULL;
static void (*s_service_callback)(DictionaryIterator *iter) = NULL;

char *command_type_to_string(CommandType command)
{
  switch (command)
  {
  case COMMAND_TYPE_STATION_LIST:
    return "stationList";
  case COMMAND_TYPE_DEPARTURES:
    return "departures";
  case COMMAND_TYPE_SERVICE:
    return "serviceInfo";
  default:
    return "unknown";
  }
}

void send_data_request(CommandType command, char *data, uint8_t data_length)
{
  // If we're not ready, cache the command and send it when we are ready
  if (!js_ready)
  {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Caching data request till js ready: %d", command);
    cached_command = command;
    return;
  }

  DictionaryIterator *iter;

  AppMessageResult result = app_message_outbox_begin(&iter);
  if (result != APP_MSG_OK)
  {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Failed to send data request: %d", result);
    return;
  }

  if (data && data_length > 0)
  {
    dict_write_cstring(iter, MESSAGE_KEY_requestData, data);
  }

  dict_write_cstring(iter, MESSAGE_KEY_request, command_type_to_string(command));

  result = app_message_outbox_send();

  if (result != APP_MSG_OK)
  {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Failed to send data request outbox: %d", result);
  }
}

// ------ APP MESSAGE CALLBACKS ------

void cached_command_send()
{
  if (cached_command)
  {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Sending cached data request: %d", cached_command);
    send_data_request(cached_command, NULL, 0);
    cached_command = COMMAND_TYPE_UNKNOWN;
  }
}

// Queues the cached command to be sent after a delay
void cached_command_queue()
{
  app_timer_register(CACHE_COMMAND_DELAY_MS, cached_command_send, NULL);
}

void js_ready_callback(bool ready)
{
  APP_LOG(APP_LOG_LEVEL_INFO, "JS READY = %d", ready);
  js_ready = ready;

  // If we have a cached command, queue the send
  if (cached_command)
  {
    cached_command_queue();
  }
}

void inbox_received_callback(DictionaryIterator *iter, void *context)
{
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Inbox received");

  Tuple *jsReady = dict_find(iter, MESSAGE_KEY_jsReady);

  if (jsReady)
  {
    js_ready_callback(jsReady->value->int32 > 0);
  }

  Tuple *objectType = dict_find(iter, MESSAGE_KEY_objectType);
  if (objectType)
  {
    if (strcmp(objectType->value->cstring, "stationList") == 0 && s_closest_station_callback != NULL)
    {
      s_closest_station_callback(iter);
    }
    else if (strcmp(objectType->value->cstring, "departureList") == 0 && s_departures_callback != NULL)
    {
      s_departures_callback(iter);
    }
    else if (strcmp(objectType->value->cstring, "serviceInfo") == 0 && s_service_callback != NULL)
    {
      s_service_callback(iter);
    }
  }
}

// ------ END APP MESSAGE CALLBACKS ------

void data_init()
{
  app_message_open(inbox_size, outbox_size);
  app_message_register_inbox_received(inbox_received_callback);
}

void request_closest_stations()
{
  send_data_request(COMMAND_TYPE_STATION_LIST, NULL, 0);
}

void request_departures(char *crs)
{
  send_data_request(COMMAND_TYPE_DEPARTURES, crs, strlen(crs));
}

void request_service(char *service_id)
{
  send_data_request(COMMAND_TYPE_SERVICE, service_id, strlen(service_id));
}

void set_closest_station_callback(void (*callback)(DictionaryIterator *iter))
{
  s_closest_station_callback = callback;
}

void set_departures_callback(void (*callback)(DictionaryIterator *iter))
{
  s_departures_callback = callback;
}

void set_service_callback(void (*callback)(DictionaryIterator *iter))
{
  s_service_callback = callback;
}
