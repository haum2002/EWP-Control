#ifndef SMARTCOOLING_WIFI_PROVISIONING_MANAGER_SHIM_H
#define SMARTCOOLING_WIFI_PROVISIONING_MANAGER_SHIM_H

#include <esp_err.h>
#include <esp_event.h>
#include <esp_wifi_types.h>
#include "wifi_provisioning/wifi_config.h"

#ifdef __cplusplus
extern "C" {
#endif

ESP_EVENT_DECLARE_BASE(WIFI_PROV_EVENT);

typedef enum {
  WIFI_PROV_INIT,
  WIFI_PROV_START,
  WIFI_PROV_CRED_RECV,
  WIFI_PROV_CRED_FAIL,
  WIFI_PROV_CRED_SUCCESS,
  WIFI_PROV_END,
  WIFI_PROV_DEINIT,
} wifi_prov_cb_event_t;

void wifi_prov_mgr_deinit(void);
esp_err_t wifi_prov_mgr_get_wifi_state(wifi_prov_sta_state_t *state);
esp_err_t wifi_prov_mgr_get_wifi_disconnect_reason(wifi_prov_sta_fail_reason_t *reason);
esp_err_t wifi_prov_mgr_configure_sta(wifi_config_t *wifi_cfg);

#ifdef __cplusplus
}
#endif

#endif
