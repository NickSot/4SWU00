#include <stddef.h>
#include <string.h>

#include "beacon_hashtable.h"

unsigned int hash_function(uint8_t beacon_mac[MAC_LEN]) {
    unsigned long hash = 5381;
    int c;
    for (size_t i = 0; i < MAC_LEN; i++) {
        c = beacon_mac[i];
        hash = ((hash << 5) + hash) + c; // hash * 33 + c (shift 5 bits then add 1 hash)
    }
    return hash % HASH_TABLE_SIZE;
}

void init_hashtable(BeaconHashTable *ht) {
    for (int i = 0; i < HASH_TABLE_SIZE; i++) {
        ht->table[i].occupied = false;
    }
}

bool insert_beacon(BeaconHashTable *ht, BeaconData data) {
    unsigned int index = hash_function(data.beacon_mac);
    for (int i = 0; i < HASH_TABLE_SIZE; i++) {
        int try = (index + i) % HASH_TABLE_SIZE; // linear probing
        if (!ht->table[try].occupied) {
            ht->table[try] = data;
            ht->table[try].occupied = true;
            return true;
        }
    }
    return false; // table is full
}

BeaconData *get_beacon(BeaconHashTable *ht, uint8_t beacon_mac[MAC_LEN]) {
    unsigned int index = hash_function(beacon_mac);
    for (int i = 0; i < HASH_TABLE_SIZE; i++) {
        int try = (index + i) % HASH_TABLE_SIZE;
        if (ht->table[try].occupied && memcmp(ht->table[try].beacon_mac, beacon_mac, MAC_LEN) == 0) {
            return &(ht->table[try]);
        }
    }
    return NULL; // Not found
}

#include "esp_log.h"
#include "esp_mac.h"

void print_hashtable(BeaconHashTable *ht) {
    ESP_LOGI("hashtable", "Hash Table {");
    for (int i = 0; i < HASH_TABLE_SIZE; i++) {
        BeaconData data = ht->table[i];
        if (data.occupied) {
            ESP_LOGI("hashtable", "\tBeacon: " MACSTR " ESP: " MACSTR " Last Ping: %ld RSSI: %d dbm",
                     MAC2STR(data.beacon_mac),
                     MAC2STR(data.esp_mac),
                     data.last_ping,
                     data.signal_strength);
        }
    }
    ESP_LOGI("hashtable", "}");
}