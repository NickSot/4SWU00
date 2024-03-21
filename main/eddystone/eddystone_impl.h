#pragma once

#include "esp_bt.h"
#include "esp_bt_defs.h"
#include "esp_bt_main.h"
#include "esp_gap_ble_api.h"
#include "esp_gatt_defs.h"
#include "esp_gattc_api.h"

#include "data_structures/beacon_hashtable.h"
#include "esp_eddystone_api.h"

void esp_eddystone_init(BeaconHashTable *const ht, uint8_t (*const bt_mac)[MAC_LEN]);
void esp_gap_cb(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param);
void esp_eddystone_show_inform(const esp_eddystone_result_t *res);