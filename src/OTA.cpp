#include "OTA.h"
#include <esp_log.h>
#include <esp_err.h>

#define OTA_BUF_SIZE 256
static const char *TAG = "OTA";

void OTA::begin()
{

  ESP_LOGI(TAG, "Starting OTA...");
  update_partition = esp_ota_get_next_update_partition(NULL);
  if (update_partition == NULL)
  {
    ESP_LOGE(TAG, "Passive OTA partition not found");
    throw ESP_FAIL;
  }
  ESP_LOGI(TAG, "Writing to partition subtype %d at offset 0x%x",
           update_partition->subtype, update_partition->address);

  esp_err_t err = esp_ota_begin(update_partition, OTA_SIZE_UNKNOWN, &update_handle);
  if (err != ESP_OK)
  {
    ESP_LOGE(TAG, "esp_ota_begin failed, error=%d", err);
    throw err;
  }
  ESP_LOGI(TAG, "esp_ota_begin succeeded");
  ESP_LOGI(TAG, "Please Wait. This may take time");
}

void OTA::write(const void *data, size_t size)
{
  esp_err_t ota_write_err = esp_ota_write(update_handle, data, size);
  if (ota_write_err != ESP_OK)
  {
    ESP_LOGE(TAG, "Error: esp_ota_write failed! err=0x%d", ota_write_err);
    throw ota_write_err;
  }
  ESP_LOGD(TAG, "Written chunk length %d", size);
}
void OTA::end()
{
  esp_err_t ota_end_err = esp_ota_end(update_handle);
  if (ota_end_err != ESP_OK)
  {
    ESP_LOGE(TAG, "Error: esp_ota_end failed! err=0x%d. Image is invalid", ota_end_err);
    throw ota_end_err;
  }

  esp_err_t err = esp_ota_set_boot_partition(update_partition);
  if (err != ESP_OK)
  {
    ESP_LOGE(TAG, "esp_ota_set_boot_partition failed! err=0x%d", err);
    throw err;
  }
  ESP_LOGI(TAG, "esp_ota_set_boot_partition succeeded");
}
OTA::OTA(MQTT &mqtt)
{
  mqtt.subscribe("/ota/data", 0, [this](const esp_mqtt_event_handle_t event) {
    if (event->current_data_offset == 0)
    {
      begin();
    }
    write(event->data, event->data_len);
    if (event->current_data_offset + event->data_len == event->total_data_len)
    {
      end();
      esp_restart();
    }
  });
}