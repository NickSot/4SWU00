#ifndef TILE_ROOM_MAPPING_H
#define TILE_ROOM_MAPPING_H

#include "stdint.h"

#define MAX_NEIGHBORS 4
#define MAX_ESP_DEVICES 10 // Maximum number of receivers
#define MAC_LEN 6

typedef struct RoomNode {
    uint8_t roomID;
    uint8_t current_visitor_count;
    uint8_t room_capacity;
    float room_occupancy;
    struct RoomNode *neighbors[MAX_NEIGHBORS];
} RoomNode;

typedef struct {
    uint8_t esp_mac[MAC_LEN]; // ESP32 MAC Address
    RoomNode *room;           // Pointer to the corresponding RoomNode
} EspRoomMapping;

RoomNode *find_room_from_esp_mac(uint8_t esp_mac[MAC_LEN]);
void initialize_esp_room_mappings();
void initialize_rooms();
void add_neighbor(RoomNode *room, RoomNode *neighbor);
float compute_occupancy_rate(RoomNode *room);
void update_room_visitor_count(uint8_t esp_mac[MAC_LEN], bool increment);

#endif
