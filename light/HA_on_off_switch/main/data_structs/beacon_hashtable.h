#ifndef BEACON_HASH_TABLE_H
#define BEACON_HASH_TABLE_H

#define UUID_LENGTH 37 // UUID string length including null terminator
#define HASH_TABLE_SIZE 256 // adjustable based on no of visitors

typedef struct {
    char uuid[UUID_LENGTH];
    int signal_strength;
    unsigned long last_pinged;
    char receiver_MAC[18]; // MAC Address string length
    int occupied;
} BeaconData;

typedef struct {
    BeaconData table[HASH_TABLE_SIZE];
} BeaconHashTable;

unsigned int hash_function(char* uuid);
void init_hashtable(BeaconHashTable* ht);
int insert_beacon(BeaconHashTable* ht, BeaconData data);
BeaconData* get_beacon(BeaconHashTable* ht, char* uuid);

#endif