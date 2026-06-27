#ifndef SMARTCOOLING_WIFI_PROVISIONING_WIFI_CONFIG_SHIM_H
#define SMARTCOOLING_WIFI_PROVISIONING_WIFI_CONFIG_SHIM_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  WIFI_PROV_STA_CONNECTING,
  WIFI_PROV_STA_CONNECTED,
  WIFI_PROV_STA_DISCONNECTED,
} wifi_prov_sta_state_t;

typedef enum {
  WIFI_PROV_STA_AUTH_ERROR,
  WIFI_PROV_STA_AP_NOT_FOUND,
} wifi_prov_sta_fail_reason_t;

#ifdef __cplusplus
}
#endif

#endif
