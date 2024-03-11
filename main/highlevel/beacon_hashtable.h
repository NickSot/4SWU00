#ifndef BEACON_HASH_TABLE_H
#define BEACON_HASH_TABLE_H

#define HASH_TABLE_SIZE 256 // adjustable based on no of visitors
#define MAC_LEN 6

#include "stdbool.h"
#include "stdint.h"

typedef struct {
    uint8_t beacon_mac[MAC_LEN];
    int16_t signal_strength;
    uint32_t last_ping;
    uint8_t esp_mac[MAC_LEN]; // MAC Address string length
    bool occupied;
} BeaconData;

typedef struct {
    BeaconData table[HASH_TABLE_SIZE];
} BeaconHashTable;

unsigned int hash_function(uint8_t beacon_mac[MAC_LEN]);
void init_hashtable(BeaconHashTable *ht);
bool insert_beacon(BeaconHashTable *ht, BeaconData data);
BeaconData *get_beacon(BeaconHashTable *ht, uint8_t beacon_mac[MAC_LEN]);
void print_hashtable(BeaconHashTable *ht);

#endif
