/*
 * SPDX-FileCopyrightText: 2021 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */

#ifndef __ESP_EDDYSTONE_PROTOCOL_H__
#define __ESP_EDDYSTONE_PROTOCOL_H__

#include "stdbool.h"
#include "stdint.h"

/* Eddystone definitions */
#define EDDYSTONE_SERVICE_UUID 0xFEAA

#define EDDYSTONE_FRAME_TYPE_TLM 0x20

// TLM
#define EDDYSTONE_TLM_VERSION_LEN 1
#define EDDYSTONE_TLM_BATTERY_VOLTAGE_LEN 2
#define EDDYSTONE_TLM_TEMPERATURE_LEN 2
#define EDDYSTONE_TLM_ADV_COUNT_LEN 4
#define EDDYSTONE_TLM_TIME_LEN 4
#define EDDYSTONE_TLM_DATA_LEN (EDDYSTONE_TLM_VERSION_LEN + EDDYSTONE_TLM_BATTERY_VOLTAGE_LEN + \
                                EDDYSTONE_TLM_TEMPERATURE_LEN + EDDYSTONE_TLM_ADV_COUNT_LEN + EDDYSTONE_TLM_TIME_LEN)

/* Eddystone TLM frame */
typedef struct {
    uint8_t version;    /*<! TLM version,0x00 for now */
    uint16_t batt;      /*<! battery voltage, 1mV/bit */
    uint16_t temp;      /*<! beacon temperature */
    uint32_t adv_count; /*<! adv pdu count since power-on or reboot */
    uint32_t time;      /*<! time sence power-on or reboot, a 0.1 second resolution counter */
} __attribute__((packed)) esp_eddystone_tlm_t;

/*  AD Structure of flags */
typedef struct {
    uint8_t len;
    uint8_t type;
    uint8_t flags;
} __attribute__((packed)) esp_eddystone_flags_t;

/* AD Structure of complete 16-bit service uuid */
typedef struct {
    uint8_t len;
    uint8_t type;
    uint16_t uuid; /*<! complete list of 16-bit service UUIDs data type value */
} __attribute__((packed)) esp_eddystone_uuid_t;

/* AD Structure of eddystone frame*/
typedef struct {
    uint8_t len;   /*<! length of eddystone data */
    uint8_t type;  /*<! service data type,must be 0x16 */
    uint16_t uuid; /*<! 16-bit eddystone uuid */
    uint8_t frame_type;
    esp_eddystone_tlm_t tlm;
} __attribute__((packed)) esp_eddystone_frame_t;

/* eddystone packet type */
typedef struct {
    esp_eddystone_flags_t flags;
    esp_eddystone_uuid_t uuid;
    esp_eddystone_frame_t frame;
} __attribute__((packed)) esp_eddystone_packet_t;

#endif /* __ESP_EDDYSTONE_PROTOCOL_H__ */
