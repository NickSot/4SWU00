/*
 * SPDX-FileCopyrightText: 2021 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */

/****************************************************************************
 *
 * This file is used to decode eddystone information.
 *
 ****************************************************************************/

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "esp_eddystone_api.h"
#include "esp_eddystone_protocol.h"
#include "esp_err.h"
#include "esp_gap_ble_api.h"

/* Declare static functions */
static esp_err_t esp_eddystone_tlm_received(const uint8_t *buf, uint8_t len, esp_eddystone_result_t *res);
static esp_err_t esp_eddystone_get_inform(const uint8_t *buf, uint8_t len, esp_eddystone_result_t *res);

/****************** eddystone-tlm ***************
 *  Unencrypted TLM Frame Specification
Byte offset	       Field	     Description
    0	          Frame Type	 Value = 0x20
    1	           Version	     TLM version, value = 0x00
    2	           VBATT[0]	     Battery voltage, 1 mV/bit
    3	           VBATT[1]
    4	           TEMP[0]	     Beacon temperature
    5	           TEMP[1]
    6	          ADV_CNT[0]	 Advertising PDU count
    7	          ADV_CNT[1]
    8	          ADV_CNT[2]
    9	          ADV_CNT[3]
    10	          SEC_CNT[0]	 Time since power-on or reboot
    11	          SEC_CNT[1]
    12	          SEC_CNT[2]
    13	          SEC_CNT[3]
************************************************/
/* decode and store received TLM */
static esp_err_t esp_eddystone_tlm_received(const uint8_t *buf, uint8_t len, esp_eddystone_result_t *res) {
    uint8_t pos = 0;
    if (len > EDDYSTONE_TLM_DATA_LEN) {
        // ERROR:TLM too long
        return -1;
    }
    res->inform.tlm.version = buf[pos++];
    res->inform.tlm.battery_voltage = big_endian_read_16(buf, pos);
    pos += 2;
    uint16_t temp = big_endian_read_16(buf, pos);
    int8_t temp_integral = (int8_t)((temp >> 8) & 0xff);
    float temp_decimal = (temp & 0xff) / 256.0;
    res->inform.tlm.temperature = temp_integral + temp_decimal;
    pos += 2;
    res->inform.tlm.adv_count = big_endian_read_32(buf, pos);
    pos += 4;
    res->inform.tlm.time = big_endian_read_32(buf, pos);
    return 0;
}

static esp_err_t esp_eddystone_get_inform(const uint8_t *buf, uint8_t len, esp_eddystone_result_t *res) {
    static esp_err_t ret = -1;
    switch (res->common.frame_type) {
        case EDDYSTONE_FRAME_TYPE_TLM: {
            ret = esp_eddystone_tlm_received(buf, len, res);
            break;
        }
        default:
            break;
    }
    return ret;
}

esp_err_t esp_eddystone_decode(const uint8_t *buf, uint8_t len, esp_eddystone_result_t *res) {
    if (len == 0 || buf == NULL || res == NULL) {
        return -1;
    }
    uint8_t pos = 0;
    while (res->common.srv_data_type != EDDYSTONE_SERVICE_UUID) {
        pos++;
        if (pos >= len) {
            return -1;
        }
        uint8_t ad_type = buf[pos++];
        switch (ad_type) {
            case ESP_BLE_AD_TYPE_FLAG: {
                res->common.flags = buf[pos++];
                break;
            }
            case ESP_BLE_AD_TYPE_16SRV_CMPL: {
                uint16_t uuid = little_endian_read_16(buf, pos);
                if (uuid != EDDYSTONE_SERVICE_UUID) {
                    return -1;
                }
                res->common.srv_uuid = uuid;
                pos += 2;
                break;
            }
            case ESP_BLE_AD_TYPE_SERVICE_DATA: {
                uint16_t type = little_endian_read_16(buf, pos);
                pos += 2;
                uint8_t frame_type = buf[pos++];
                if (type != EDDYSTONE_SERVICE_UUID || frame_type != EDDYSTONE_FRAME_TYPE_TLM) {
                    return -1;
                }
                res->common.srv_data_type = type;
                res->common.frame_type = frame_type;
                break;
            }
            default:
                break;
        }
    }
    return esp_eddystone_get_inform(buf + pos, len - pos, res);
}
