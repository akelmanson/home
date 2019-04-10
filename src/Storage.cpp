#include "Storage.h"

Storage::Storage()
{
  esp_err_t err = nvs_flash_init();
  if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
  {
    // NVS partition was truncated and needs to be erased
    // Retry nvs_flash_init
    ESP_ERROR_CHECK(nvs_flash_erase());
    err = nvs_flash_init();
  }
  ERROR_CHECK(err);
  ERROR_CHECK(nvs_open("storage", NVS_READWRITE, &handle));
}

Storage::~Storage()
{
  nvs_close(handle);
}
