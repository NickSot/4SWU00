#ifndef BEACON_HASH_TABLE_H
#define BEACON_HASH_TABLE_H

#define HASH_TABLE_SIZE 256 // adjustable based on no of visitors
#define MAC_LEN 6

#include "stdbool.h"
#include "stdint.h"
#include "esp_mac.h"

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

typedef enum {
    /// @brief The beacon was inserted as a new row in the table
    BEACON_INSERT,
    /// @brief A beacon with the same MAC was found, and its data was updated
    BEACON_UPDATE,

    /// @brief The table was full and the beacon was not able to be added
    BEACON_DROPPED,
} BeaconUpdateStatus;

void init_hashtable(BeaconHashTable *ht);
BeaconUpdateStatus insert_update_beacon(BeaconHashTable *ht, BeaconData data);
BeaconData * get_beacon(BeaconHashTable *ht, uint8_t beacon_mac[MAC_LEN]);
void print_hashtable(BeaconHashTable *ht);
bool delete_beacon(BeaconHashTable *ht, uint8_t beacon_mac[MAC_LEN]);
void check_and_delete_stale_beacons(BeaconHashTable *ht);

#define BEACONSTR "Beacon: " MACSTR " ESP: " MACSTR " Last Ping: %ld | RSSI: %d dBm | Battery Voltage: %d mV | Temp deg C: %6.1f | adv: %ld | uptime: %ld"
#define BEACON2STR(beacon)       \
    MAC2STR(beacon.beacon_mac),  \
        MAC2STR(beacon.esp_mac), \
        beacon.last_ping,        \
        beacon.signal_strength,  \
        beacon.battery_voltage,  \
        beacon.temperature,      \
        beacon.adv_count,        \
        beacon.up_time

#endif
