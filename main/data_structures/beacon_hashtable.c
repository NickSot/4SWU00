#include "beacon_hashtable.h"
#include "esp_log.h"
#include "tile_room_mapping.h"
#include <stddef.h>
#include <string.h>

#define STALE_BEACON_THRESHOLD 300000 // Time in milliseconds

static unsigned int hash_function(uint8_t beacon_mac[MAC_LEN]) {
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

BeaconUpdateStatus insert_update_beacon(BeaconHashTable *ht, BeaconData data) {
    unsigned int index = hash_function(data.beacon_mac);
    for (int i = 0; i < HASH_TABLE_SIZE; i++) {
        int try = (index + i) % HASH_TABLE_SIZE; // linear probing
        if (ht->table[try].occupied) {
            // Check if the current slot has the same beacon MAC address
            if (memcmp(ht->table[try].data.beacon_mac, data.beacon_mac, MAC_LEN) == 0) {
                // Beacon exists, update the existing data
                ht->table[try].data = data;
                ht->table[try].occupied = true;
                return BEACON_UPDATE; // Beacon data updated
            }
        } else {
            // Slot not occupied, insert new beacon data here
            ht->table[try].data = data;
            ht->table[try].occupied = true;
            // After successfully inserting a beacon
            update_room_visitor_count(data.esp_mac, true); // Increment visitor count
            return BEACON_INSERT;                          // New beacon data inserted
        }
    }
    return BEACON_DROPPED; // table is full
}

BeaconData *get_beacon(BeaconHashTable *ht, uint8_t beacon_mac[MAC_LEN]) {
    unsigned int index = hash_function(beacon_mac);
    for (int i = 0; i < HASH_TABLE_SIZE; i++) {
        int try = (index + i) % HASH_TABLE_SIZE;
        if (ht->table[try].occupied && memcmp(ht->table[try].data.beacon_mac, beacon_mac, MAC_LEN) == 0) {
            return &(ht->table[try].data);
        }
    }
    return NULL; // Not found
}

void print_hashtable(BeaconHashTable *ht) {
    ESP_LOGI("hashtable", "Hash Table {");
    for (int i = 0; i < HASH_TABLE_SIZE; i++) {
        BeaconDataWrapper beacon_data = ht->table[i];
        BeaconData data = beacon_data.data;
        if (beacon_data.occupied) {
            ESP_LOGI("hashtable", "\t" BEACONSTR, BEACON2STR(data));
        }
    }
    ESP_LOGI("hashtable", "}");
}

bool delete_beacon(BeaconHashTable *ht, uint8_t beacon_mac[MAC_LEN]) {
    unsigned int index = hash_function(beacon_mac);
    for (int i = 0; i < HASH_TABLE_SIZE; i++) {
        int try = (index + i) % HASH_TABLE_SIZE;
        if (ht->table[try].occupied && memcmp(ht->table[try].data.beacon_mac, beacon_mac, MAC_LEN) == 0) {
            // Beacon found, mark as not occupied
            ht->table[try].occupied = false;
            // Clear the data
            memset(&ht->table[try], 0, sizeof(BeaconData));
            return true; // Deleted

        } else if (!ht->table[try].occupied) {
            // Stop if an unoccupied slot is encountered, meaning the beacon is not in the table
            break;
        }
    }
    return false; // Beacon not found
}

void check_and_delete_stale_beacons(BeaconHashTable *ht) {
    uint32_t current_time = esp_log_timestamp(); // Get current time in milliseconds since boot
    for (int i = 0; i < HASH_TABLE_SIZE; i++) {
        if (ht->table[i].occupied) {
            uint32_t time_diff = current_time - ht->table[i].data.last_ping;
            if (time_diff > STALE_BEACON_THRESHOLD) {
                update_room_visitor_count(ht->table[i].data.esp_mac, false); // Decrement visitor count
                // Beacon is considered stale, delete it
                delete_beacon(ht, ht->table[i].data.beacon_mac);
                ESP_LOGI("hashtable", "\tDeleted stale beacon: " MACSTR,
                         MAC2STR(ht->table[i].data.beacon_mac));
            }
        }
    }
}
