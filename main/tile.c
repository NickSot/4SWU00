#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "esp_mac.h"
#include "esp_system.h"
#include "nvs_flash.h"

#include "data_structures/beacon_hashtable.h"
#include "eddystone/eddystone_impl.h"
#include "zigbee/beacon_data_handle.h"
#include "zigbee/esp_zb_end_device.h"

uint8_t bt_mac[6];
BeaconHashTable ht;

static const char *TAG = "COMPASS_TILE";

static esp_ble_scan_params_t ble_scan_params = {
    .scan_type = BLE_SCAN_TYPE_ACTIVE,
    .own_addr_type = BLE_ADDR_TYPE_PUBLIC,
    .scan_filter_policy = BLE_SCAN_FILTER_ALLOW_ALL,
    .scan_interval = 0x50,
    .scan_window = 0x30,
    .scan_duplicate = BLE_SCAN_DUPLICATE_DISABLE
};

// #if !defined ZB_ED_ROLE
// #error Define ZB_ED_ROLE in idf.py menuconfig to compile light (End Device) source code.
// #endif

static void bdb_start_top_level_commissioning_cb(uint8_t mode_mask) {
    ESP_ERROR_CHECK(esp_zb_bdb_start_top_level_commissioning(mode_mask));
}

void esp_zb_app_signal_handler(esp_zb_app_signal_t *signal_struct) {
    uint32_t *p_sg_p = signal_struct->p_app_signal;
    esp_err_t err_status = signal_struct->esp_err_status;
    esp_zb_app_signal_type_t sig_type = *p_sg_p;
    switch (sig_type) {
        case ESP_ZB_ZDO_SIGNAL_SKIP_STARTUP:
            ESP_LOGI(TAG, "Zigbee stack initialized");
            ESP_ERROR_CHECK(esp_zb_bdb_start_top_level_commissioning(ESP_ZB_BDB_MODE_INITIALIZATION));
            break;
        case ESP_ZB_BDB_SIGNAL_DEVICE_FIRST_START:
        case ESP_ZB_BDB_SIGNAL_DEVICE_REBOOT:
            if (err_status == ESP_OK) {
                ESP_LOGI(TAG, "Start network steering");
                ESP_ERROR_CHECK(esp_zb_bdb_start_top_level_commissioning(ESP_ZB_BDB_MODE_NETWORK_STEERING));
            } else {
                /* commissioning failed */
                ESP_LOGW(TAG, "Failed to initialize Zigbee stack (status: %s)", esp_err_to_name(err_status));
            }
            break;
        case ESP_ZB_BDB_SIGNAL_STEERING:
            if (err_status == ESP_OK) {
                esp_zb_ieee_addr_t extended_pan_id;
                esp_zb_get_extended_pan_id(extended_pan_id);
                ESP_LOGI(TAG, "Joined network successfully (Extended PAN ID: %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x, PAN ID: 0x%04hx, Channel:%d)",
                         extended_pan_id[7], extended_pan_id[6], extended_pan_id[5], extended_pan_id[4],
                         extended_pan_id[3], extended_pan_id[2], extended_pan_id[1], extended_pan_id[0],
                         esp_zb_get_pan_id(), esp_zb_get_current_channel());

                /*<! set scan parameters */
                esp_ble_gap_set_scan_params(&ble_scan_params);
            } else {
                ESP_LOGI(TAG, "Network steering was not successful (status: %s)", esp_err_to_name(err_status));
                esp_zb_scheduler_alarm((esp_zb_callback_t)bdb_start_top_level_commissioning_cb, ESP_ZB_BDB_MODE_NETWORK_STEERING, 1000);
            }
            break;
        default:
            ESP_LOGI(TAG, "ZDO signal: %s (0x%x), status: %s", esp_zb_zdo_signal_to_string(sig_type), sig_type,
                     esp_err_to_name(err_status));
            break;
    }
}

static esp_err_t zb_attribute_handler(esp_zb_apsde_data_confirm_t *confirm) {
    ESP_LOGI(TAG, "Received message: endpoint(%d), data size(%d)",
             (int)confirm->src_endpoint, (int)confirm->asdu_length);

    ESP_ERROR_CHECK(receive_beacon_data(confirm, &ht));

    return ESP_OK;
}

static void zb_action_handler(esp_zb_apsde_data_confirm_t confirm) {
    switch (confirm.status) {
        case ESP_OK:
            // ret = zb_attribute_handler(&confirm);
            ESP_ERROR_CHECK(zb_attribute_handler(&confirm));
            break;
        default:
            ESP_LOGW(TAG, "Receive Zigbee action(0x%d) callback", confirm.status);
            break;
    }
}

static void esp_zb_task(void *pvParameters) {
    /* initialize Zigbee stack */
    esp_zb_cfg_t zb_nwk_cfg = ESP_ZB_ZED_CONFIG();
    esp_zb_init(&zb_nwk_cfg);

    esp_zb_ep_list_t *esp_zb_on_off_light_ep = esp_zb_ep_list_create();
    ESP_ERROR_CHECK(esp_zb_device_register(esp_zb_on_off_light_ep));

    esp_zb_aps_data_confirm_handler_register(zb_action_handler);
    ESP_ERROR_CHECK(esp_zb_set_primary_network_channel_set(ESP_ZB_PRIMARY_CHANNEL_MASK));
    ESP_ERROR_CHECK(esp_zb_start(false));
    esp_zb_main_loop_iteration();
}

void tile_main() {
    init_hashtable(&ht);
    ESP_LOGI(TAG, "Initialized hashtable");

    ESP_ERROR_CHECK(nvs_flash_init());

    ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));
    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_bt_controller_init(&bt_cfg));
    ESP_ERROR_CHECK(esp_bt_controller_enable(ESP_BT_MODE_BLE));

    esp_zb_platform_config_t config = {
        .radio_config = ESP_ZB_DEFAULT_RADIO_CONFIG(),
        .host_config = ESP_ZB_DEFAULT_HOST_CONFIG(),
    };
    ESP_ERROR_CHECK(esp_zb_platform_config(&config));

    ESP_ERROR_CHECK(esp_read_mac(bt_mac, ESP_MAC_BT));
    ESP_LOGI("museum_compass", "Bluetooth MAC: " MACSTR, MAC2STR(bt_mac));

    esp_eddystone_init(&ht, &bt_mac);

    /*<! set scan parameters */
    // esp_ble_gap_set_scan_params(&ble_scan_params);

    xTaskCreate(esp_zb_task, "Zigbee_main", 4096, NULL, 5, NULL);
}