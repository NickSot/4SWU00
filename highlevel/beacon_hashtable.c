#include "beacon_hashtable.h"
#include <stddef.h> 
#include <stdlib.h> 
#include <string.h> 

unsigned int hash_function(char* uuid) {
    unsigned long hash = 5381;
    int c;
    while ((c = *uuid++)) {
        hash = ((hash << 5) + hash) + c; // hash * 33 + c (shift 5 bits then add 1 hash)
    }
    return hash % HASH_TABLE_SIZE;
}

void init_hashtable(BeaconHashTable* ht) {
    for (int i = 0; i < HASH_TABLE_SIZE; i++) {
        ht->table[i].occupied = 0;
    }
}

int insert_beacon(BeaconHashTable* ht, BeaconData data) {
    unsigned int index = hash_function(data.uuid);
    for (int i = 0; i < HASH_TABLE_SIZE; i++) {
        int try = (index + i) % HASH_TABLE_SIZE; // linear probing
        if (!ht->table[try].occupied) {
            ht->table[try] = data;
            ht->table[try].occupied = 1;
            return 1;
        }
    }
    return 0; //table is full
}

BeaconData* get_beacon(BeaconHashTable* ht, char* uuid) {
    unsigned int index = hash_function(uuid);
    for (int i = 0; i < HASH_TABLE_SIZE; i++) {
        int try = (index + i) % HASH_TABLE_SIZE;
        if (ht->table[try].occupied && strcmp(ht->table[try].uuid, uuid) == 0) {
            return &(ht->table[try]);
        }
    }
    return NULL; // Not found
}
