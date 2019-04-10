#include <stdint.h>
#include <nvs_flash.h>
#include "common.h"

#define STORAGE_GETTER(suffix, type)                                   \
  type get_##suffix(const char *key)                                   \
  {                                                                    \
    type value;                                                        \
    ERROR_CHECK(nvs_get_##suffix(handle, key, &value));                \
    return value;                                                      \
  }                                                                    \
  type get_##suffix(const char *key, type default_value)               \
  {                                                                    \
    try                                                                \
    {                                                                  \
      return get_##suffix(key);                                        \
    }                                                                  \
    catch (esp_err_t err)                                              \
    {                                                                  \
      return err == ESP_ERR_NVS_NOT_FOUND ? default_value : throw err; \
    }                                                                  \
  }

#define STORAGE_SETTER(suffix, type)                   \
  void set_##suffix(const char *key, type value)       \
  {                                                    \
    ERROR_CHECK(nvs_set_##suffix(handle, key, value)); \
  }

#define STORAGE_ACCESSOR(suffix, type) \
  STORAGE_GETTER(suffix, type)         \
  STORAGE_SETTER(suffix, type)

class Storage
{
protected:
  nvs_handle handle;

public:
  Storage();
  ~Storage();
  STORAGE_ACCESSOR(u32, uint32_t);
};

#undef STORAGE_ACCESSOR
#undef STORAGE_GETTER
#undef STORAGE_SETTER