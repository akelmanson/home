#pragma once

#include "mqtt_client.h"
// #include "test.h"
#include "esp_system.h"
#include "WiFi.h"
#include <functional>
#include <list>

const std::string NO_PENDING_TOPIC;

typedef std::function<void(const esp_mqtt_event_handle_t)> subscriber_t;

struct Subscription
{
  std::string topic;
  int qos;
  subscriber_t subscriber;
};

class MQTT
{
protected:
  esp_mqtt_client *client;
  bool connected = false;
  std::list<Subscription> subscriptions;
  std::string pending_topic = NO_PENDING_TOPIC;

  esp_err_t event_handler(esp_mqtt_event_handle_t event);
  void notify_subscribers(const char *topic, int topic_len, esp_mqtt_event_handle_t event);

public:
  MQTT(WiFi &wifi, const char *url);
  void subscribe(const char *topic, int qos, subscriber_t callback);
  bool is_connected();
  void publish(const char *topic, const char *data, int len, int qos, bool retain);
  void publish(const char *topic, std::string &text, int qos, bool retain);
};
