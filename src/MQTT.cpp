#include "MQTT.h"

#include "esp_log.h"
#include <esp_task.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/FreeRTOSConfig.h>

#define IS_MULTIPART(event) (event->data_len < event->total_data_len)
#define IS_FIRST_PART(event) (event->current_data_offset == 0)
#define IS_LAST_PART(event) (event->current_data_offset + event->data_len == event->total_data_len)

static const char *TAG = "MQTT_EXAMPLE";

void MQTT::notify_subscribers(const char *topic, int topic_len, esp_mqtt_event_handle_t event)
{
  for (auto &subscription : subscriptions)
  {
    if (subscription.topic.compare(0, topic_len, topic) == 0)
    {
      subscription.subscriber(event);
    }
  }
}

void MQTT::publish(const char *topic, const char *data, int len, int qos, bool retain)
{
  esp_mqtt_client_publish(client, topic, data, len, qos, retain);
}

void MQTT::publish(const char *topic, std::string &text, int qos, bool retain)
{
  publish(topic, text.c_str(), 0, qos, retain);
}

esp_err_t MQTT::event_handler(esp_mqtt_event_handle_t event)
{
  esp_mqtt_client_handle_t client = event->client;
  int msg_id;
  switch (event->event_id)
  {
  case MQTT_EVENT_CONNECTED:
    connected = true;
    for (auto &subscription : subscriptions)
    {
      esp_mqtt_client_subscribe(client, subscription.topic.c_str(), subscription.qos);
    }
    break;
  case MQTT_EVENT_DISCONNECTED:
    connected = false;
    break;

  case MQTT_EVENT_SUBSCRIBED:
    ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
    msg_id = esp_mqtt_client_publish(client, "/topic/qos0", "data", 0, 0, 0);
    ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
    break;
  case MQTT_EVENT_UNSUBSCRIBED:
    ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
    break;
  case MQTT_EVENT_PUBLISHED:
    ESP_LOGD(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
    break;
  case MQTT_EVENT_DATA:
    if (!IS_MULTIPART(event))
    {
      notify_subscribers(event->topic, event->topic_len, event);
    }
    else
    {
      if (IS_FIRST_PART(event))
      {
        pending_topic = std::string(event->topic, event->topic_len);
      }
      notify_subscribers(pending_topic.c_str(), pending_topic.length(), event);
      if (IS_LAST_PART(event))
      {
        pending_topic = NO_PENDING_TOPIC;
      }
    }
    break;
  case MQTT_EVENT_ERROR:
    ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
    break;
  }
  return ESP_OK;
}
bool MQTT::is_connected()
{
  return connected;
}
void MQTT::subscribe(const char *topic, int qos, subscriber_t callback)
{
  subscriptions.push_back(Subscription{
      .topic = topic,
      .qos = qos,
      .subscriber = callback,
  });
  if (is_connected())
  {
    esp_mqtt_client_subscribe(client, topic, qos);
  }
}
MQTT::MQTT(WiFi &wifi, const char *url)
{
  wifi.wait_to_connect();
  esp_mqtt_client_config_t mqtt_cfg = {};
  mqtt_cfg.uri = url;
  mqtt_cfg.event_handle = [](esp_mqtt_event_handle_t event) {
    return ((MQTT *)event->user_context)->event_handler(event);
  };
  mqtt_cfg.user_context = this;
  client = esp_mqtt_client_init(&mqtt_cfg);
  esp_mqtt_client_start(client);
}
