#include "eddystone_impl.h"
#include "esp_bt.h"
#include "esp_bt_defs.h"
#include "esp_bt_main.h"
#include "esp_gap_ble_api.h"
#include "esp_gatt_defs.h"
#include "esp_gattc_api.h"
#include "esp_log.h"

#include "data_structures/beacon_hashtable.h"

static uint8_t (*bluetooth_mac)[MAC_LEN];
static BeaconHashTable *hash_table;

void esp_eddystone_show_inform(const esp_eddystone_result_t *res) {
    switch (res->common.frame_type) {
        case EDDYSTONE_FRAME_TYPE_TLM: {
            ESP_LOGI(__FILE__, "Eddystone TLM inform:");
            ESP_LOGI(__FILE__, "version: %d", res->inform.tlm.version);
            ESP_LOGI(__FILE__, "battery voltage: %d mV", res->inform.tlm.battery_voltage);
            ESP_LOGI(__FILE__, "beacon temperature in degrees Celsius: %6.1f", res->inform.tlm.temperature);
            ESP_LOGI(__FILE__, "adv pdu count since power-up: %" PRIu32, res->inform.tlm.adv_count);
            ESP_LOGI(__FILE__, "time since power-up: %" PRIu32 " s", (res->inform.tlm.time) / 10);
            break;
        }
        default:
            break;
    }
}

void esp_gap_cb(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param) {
    esp_err_t err;

    switch (event) {
        case ESP_GAP_BLE_SCAN_PARAM_SET_COMPLETE_EVT: {
            uint32_t duration = 0;
            esp_ble_gap_start_scanning(duration);
            break;
        }
        case ESP_GAP_BLE_SCAN_START_COMPLETE_EVT: {
            if ((err = param->scan_start_cmpl.status) != ESP_BT_STATUS_SUCCESS) {
                ESP_LOGE(__FILE__, "Scan start failed: %s", esp_err_to_name(err));
            } else {
                ESP_LOGI(__FILE__, "Start scanning...");
            }
            break;
        }
        case ESP_GAP_BLE_SCAN_RESULT_EVT: {
            esp_ble_gap_cb_param_t *scan_result = (esp_ble_gap_cb_param_t *)param;
            switch (scan_result->scan_rst.search_evt) {
                case ESP_GAP_SEARCH_INQ_RES_EVT: {
                    esp_eddystone_result_t eddystone_res;
                    memset(&eddystone_res, 0, sizeof(eddystone_res));
                    esp_err_t ret = esp_eddystone_decode(scan_result->scan_rst.ble_adv, scan_result->scan_rst.adv_data_len, &eddystone_res);
                    if (ret) {
                        // error:The received data is not an eddystone frame packet or a correct eddystone frame packet.
                        // just return
                        return;
                    } else {
                        // The received adv data is a correct eddystone frame packet.
                        // Here, we get the eddystone infomation in eddystone_res, we can use the data in res to do other things.
                        // For example, just print them:
                        // ESP_LOGI(DEMO_TAG, "--------Eddystone Found----------");
                        // esp_log_buffer_hex("EDDYSTONE_DEMO: Device address:", scan_result->scan_rst.bda, ESP_BD_ADDR_LEN);
                        // ESP_LOGI(DEMO_TAG, "RSSI of packet:%d dbm", scan_result->scan_rst.rssi);
                        // esp_eddystone_show_inform(&eddystone_res);

                        BeaconData data;
                        memcpy(data.beacon_mac, scan_result->scan_rst.bda, 6);
                        memcpy(data.esp_mac, bluetooth_mac, 6);
                        data.signal_strength = scan_result->scan_rst.rssi;
                        data.battery_voltage = eddystone_res.inform.tlm.battery_voltage;
                        data.temperature = eddystone_res.inform.tlm.temperature;
                        data.last_ping = esp_log_timestamp();
                        data.adv_count = eddystone_res.inform.tlm.adv_count;
                        data.up_time = eddystone_res.inform.tlm.time;

                        int result = insert_update_beacon(hash_table, data);
                        if (result == 1) {
                            ESP_LOGI(__FILE__, "Beacon inserted: %d", true);
                        } else if (result == 2) {
                            ESP_LOGI(__FILE__, "Beacon updated: %d", true);
                        } else if (result == 0) {
                            ESP_LOGI(__FILE__, "Beacon processed: %d", false);
                        }

                        print_hashtable(hash_table);
                    }
                    break;
                }
                default:
                    break;
            }
            break;
        }
        case ESP_GAP_BLE_SCAN_STOP_COMPLETE_EVT: {
            if ((err = param->scan_stop_cmpl.status) != ESP_BT_STATUS_SUCCESS) {
                ESP_LOGE(__FILE__, "Scan stop failed: %s", esp_err_to_name(err));
            } else {
                ESP_LOGI(__FILE__, "Stop scan successfully");
            }
            break;
        }
        default:
            break;
    }
}

void esp_eddystone_appRegister(void) {
    esp_err_t status;

    ESP_LOGI(__FILE__, "Register callback");

    /*<! register the scan callback function to the gap module */
    if ((status = esp_ble_gap_register_callback(esp_gap_cb)) != ESP_OK) {
        ESP_LOGE(__FILE__, "gap register error: %s", esp_err_to_name(status));
        return;
    }
}

void esp_eddystone_init(BeaconHashTable *const ht, uint8_t  (*const bt_mac)[MAC_LEN]) {
    hash_table = ht;
    bluetooth_mac = bt_mac;

    esp_bluedroid_config_t bluedroid_cfg = BT_BLUEDROID_INIT_CONFIG_DEFAULT();
    esp_bluedroid_init_with_cfg(&bluedroid_cfg);
    esp_bluedroid_enable();
    esp_eddystone_appRegister();
}