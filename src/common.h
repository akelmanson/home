#include <esp_err.h>

#define ERROR_CHECK(x)                                                     \
  do                                                                       \
  {                                                                        \
    esp_err_t __err_rc = (x);                                              \
    if (__err_rc != ESP_OK)                                                \
    {                                                                      \
      printf("%s:%d %s\n", __FILE__, __LINE__, esp_err_to_name(__err_rc)); \
      throw(__err_rc);                                                     \
    }                                                                      \
  } while (0);
