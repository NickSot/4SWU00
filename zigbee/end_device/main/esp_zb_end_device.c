/*
 * SPDX-FileCopyrightText: 2021-2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: LicenseRef-Included
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form, except as embedded into a Espressif Systems
 *    integrated circuit in a product or a software update for such product,
 *    must reproduce the above copyright notice, this list of conditions and
 *    the following disclaimer in the documentation and/or other materials
 *    provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 *    may be used to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * 4. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "string.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_check.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "aps/esp_zigbee_aps.h"
#include "esp_zb_end_device.h"

#if !defined ZB_ED_ROLE
#error Define ZB_ED_ROLE in idf.py menuconfig to compile light (End Device) source code.
#endif

static const char *TAG = "ESP_ZB_ON_OFF_LIGHT";

esp_zb_apsde_data_req_t serialize_beacon_data(BeaconData * beacon_data_ptr) {
    uint8_t buffer[sizeof(BeaconData)];

    memcpy(buffer, beacon_data_ptr, sizeof(BeaconData));

    esp_zb_apsde_data_req_t cmd_req;

    cmd_req.dst_addr_mode = ESP_ZB_APS_ADDR_MODE_16_ENDP_PRESENT;
    cmd_req.dst_endpoint = 0xff;
    cmd_req.asdu_length = sizeof(buffer);
    cmd_req.asdu = buffer;
    cmd_req.cluster_id = 1;
    cmd_req.profile_id = 1;
    
    cmd_req.dst_short_addr = 0xffff;
    cmd_req.use_alias = false;
    cmd_req.radius = 0x5;
    cmd_req.src_endpoint = esp_zb_get_short_address();
    cmd_req.tx_options = ESP_ZB_APSDE_TX_OPT_NO_LONG_ADDR;

    ESP_EARLY_LOGI(TAG, "Send serialized composite data structure");

    return cmd_req;
}

void process_beacon_data(BeaconData data) {

}

BeaconData deserialize_beacon_data(u_int8_t * buffer) {
    BeaconData beacon_data;
    memcpy(&beacon_data, buffer, sizeof(BeaconData));

    process_beacon_data(beacon_data);

    return beacon_data;
}

/********************* Define functions **************************/
static void bdb_start_top_level_commissioning_cb(uint8_t mode_mask)
{
    ESP_ERROR_CHECK(esp_zb_bdb_start_top_level_commissioning(mode_mask));
}

void esp_zb_app_signal_handler(esp_zb_app_signal_t *signal_struct)
{
    uint32_t *p_sg_p       = signal_struct->p_app_signal;
    esp_err_t err_status = signal_struct->esp_err_status;
    esp_zb_app_signal_type_t sig_type = *p_sg_p;
    switch (sig_type) {
    case ESP_ZB_ZDO_SIGNAL_SKIP_STARTUP:
        ESP_LOGI(TAG, "Zigbee stack initialized");
        esp_zb_bdb_start_top_level_commissioning(ESP_ZB_BDB_MODE_INITIALIZATION);
        break;
    case ESP_ZB_BDB_SIGNAL_DEVICE_FIRST_START:
    case ESP_ZB_BDB_SIGNAL_DEVICE_REBOOT:
        if (err_status == ESP_OK) {
            ESP_LOGI(TAG, "Start network steering");
            esp_zb_bdb_start_top_level_commissioning(ESP_ZB_BDB_MODE_NETWORK_STEERING);
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

static esp_err_t zb_attribute_handler(esp_zb_apsde_data_confirm_t * confirm)
{
    ESP_LOGI(TAG, "Received message: endpoint(%d), data size(%d)",
            (int)confirm->src_endpoint, (int)confirm->asdu_length);
    
    BeaconData data = deserialize_beacon_data(confirm->asdu);

    ESP_LOGI(TAG, "receiver MAC address: %s", data.receiver_MAC);
    ESP_LOGI(TAG, "beacon UUID: %s", data.uuid);
    ESP_LOGI(TAG, "receiver signal strength: %d", data.signal_strength);

    return ESP_OK;
}

static esp_err_t zb_attribute_ind_handler(esp_zb_apsde_data_ind_t * confirm)
{

    ESP_LOGI(TAG, "Received message: endpoint(%d), data size(%d)",
            (int)confirm->src_endpoint, (int)confirm->asdu_length);
    
    BeaconData data = deserialize_beacon_data(confirm->asdu);

    ESP_LOGI(TAG, "receiver MAC address: %s", data.receiver_MAC);
    ESP_LOGI(TAG, "beacon UUID: %s", data.uuid);
    ESP_LOGI(TAG, "receiver signal strength: %d", data.signal_strength);

    return ESP_OK;
}

static void zb_action_handler(esp_zb_apsde_data_confirm_t confirm)
{
    switch (confirm.status) {
        case ESP_OK:
            // ret = zb_attribute_handler(&confirm);
            zb_attribute_handler(&confirm);
            break;
        default:
            ESP_LOGW(TAG, "Receive Zigbee action(0x%d) callback", confirm.status);
            break;
    }
}

static bool zb_action_handler_ind(esp_zb_apsde_data_ind_t ind)
{
    bool ret = false;

    switch (ind.status) {
        case ESP_OK:
            // ret = zb_attribute_handler(&confirm);
            zb_attribute_ind_handler(&ind);
            ret = true;
            break;

        default:
            ESP_LOGW(TAG, "Receive Zigbee action(0x%d) callback", ind.status);
            break;
    }
    
    return ret;
}

static void esp_zb_task(void *pvParameters)
{
    /* initialize Zigbee stack */
    esp_zb_cfg_t zb_nwk_cfg = ESP_ZB_ZED_CONFIG();
    esp_zb_init(&zb_nwk_cfg);
    // esp_zb_on_off_light_cfg_t light_cfg = ESP_ZB_DEFAULT_ON_OFF_LIGHT_CONFIG();
    // esp_zb_ep_list_t *esp_zb_on_off_light_ep = esp_zb_on_off_light_ep_create(HA_ESP_LIGHT_ENDPOINT, &light_cfg);
    esp_zb_ep_list_t *esp_zb_on_off_light_ep = esp_zb_ep_list_create();
    esp_zb_device_register(esp_zb_on_off_light_ep);
    // esp_zb_core_action_handler_register(zb_action_handler);
    esp_zb_aps_data_confirm_handler_register(zb_action_handler);
    esp_zb_aps_data_indication_handler_register(zb_action_handler_ind);
    esp_zb_set_primary_network_channel_set(ESP_ZB_PRIMARY_CHANNEL_MASK);
    ESP_ERROR_CHECK(esp_zb_start(false));
    esp_zb_main_loop_iteration();
}

void app_main(void)
{
    esp_zb_platform_config_t config = {
        .radio_config = ESP_ZB_DEFAULT_RADIO_CONFIG(),
        .host_config = ESP_ZB_DEFAULT_HOST_CONFIG(),
    };
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_zb_platform_config(&config));
    light_driver_init(LIGHT_DEFAULT_OFF);
    xTaskCreate(esp_zb_task, "Zigbee_main", 4096, NULL, 5, NULL);
}
