#ifndef BEACON_HASH_TABLE_H
#define BEACON_HASH_TABLE_H

#define HASH_TABLE_SIZE 256 // adjustable based on no of visitors
#define MAC_LEN 6

#include "stdbool.h"
#include "stdint.h"

typedef struct {
    uint8_t beacon_mac[MAC_LEN];
    uint8_t esp_mac[MAC_LEN]; // MAC Address string length
    int16_t signal_strength;
    int16_t battery_voltage; // in millivolts
    float temperature;
    uint32_t last_ping;
    uint32_t adv_count; // advertisement packet count since power-up
    uint32_t up_time;   // time since power-up in seconds
} BeaconData;

typedef struct {
    bool occupied;
    BeaconData data;
} BeaconDataWrapper;

typedef struct {
    BeaconDataWrapper table[HASH_TABLE_SIZE];
} BeaconHashTable;

unsigned int hash_function(uint8_t beacon_mac[MAC_LEN]);
void init_hashtable(BeaconHashTable *ht);
int insert_update_beacon(BeaconHashTable *ht, BeaconData data);
BeaconData *get_beacon(BeaconHashTable *ht, uint8_t beacon_mac[MAC_LEN]);
void print_hashtable(BeaconHashTable *ht);
bool delete_beacon(BeaconHashTable *ht, uint8_t beacon_mac[MAC_LEN]);
void check_and_delete_stale_beacons(BeaconHashTable *ht);

#endif
