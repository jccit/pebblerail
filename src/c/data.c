#include "data.h"

const uint32_t inbox_size = 512;
const uint32_t outbox_size = 256;
const uint32_t CACHE_COMMAND_DELAY_MS = 500;

static bool js_ready = false;
static CommandType cached_command = COMMAND_TYPE_UNKNOWN;

static void (*s_closest_station_callback)(DictionaryIterator *iter) = NULL;

char *command_type_to_string(CommandType command)
{
  switch (command)
  {
  case COMMAND_TYPE_STATION_LIST:
    return "stationList";
  default:
    return "unknown";
  }
}

void send_data_request(CommandType command)
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

  dict_write_cstring(iter, MESSAGE_KEY_dataRequest, command_type_to_string(command));

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
    send_data_request(cached_command);
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

  Tuple *stationList = dict_find(iter, MESSAGE_KEY_stationList);
  if (stationList && s_closest_station_callback != NULL)
  {
    s_closest_station_callback(iter);
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
  send_data_request(COMMAND_TYPE_STATION_LIST);
}

void set_closest_station_callback(void (*callback)(DictionaryIterator *iter))
{
  s_closest_station_callback = callback;
}