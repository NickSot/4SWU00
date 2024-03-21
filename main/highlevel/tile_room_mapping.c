#include <stddef.h>
#include <string.h>

#include "tile_room_mapping.h"

#define NUM_ROOMS 4

RoomNode rooms[NUM_ROOMS];
EspRoomMapping esp_room_mapping[NUM_ROOMS];

void initialize_rooms() {
    for (int i = 0; i < NUM_ROOMS; i++) {
        rooms[i].roomID = i + 1;
        rooms[i].current_visitor_count = 0;
        rooms[i].room_capacity = 10; // Example of capacity
        rooms[i].room_occupancy = 0.0;
    }
}

void initialize_esp_room_mappings() {
    // Example MAC addresses
    uint8_t macs[NUM_ROOMS][MAC_LEN] = {
        { 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0x01 },
        { 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0x02 },
        { 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0x03 },
        { 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0x04 }
    };

    for (int i = 0; i < NUM_ROOMS; i++) {
        memcpy(esp_room_mapping[i].esp_mac, macs[i], MAC_LEN);
        esp_room_mapping[i].room = &rooms[i]; // Link each MAC address to a room
    }
}

RoomNode *find_room_from_esp_mac(uint8_t esp_mac[MAC_LEN]) {
    for (int i = 0; i < NUM_ROOMS; i++) {
        if (memcmp(esp_room_mapping[i].esp_mac, esp_mac, MAC_LEN) == 0) {
            return esp_room_mapping[i].room;
        }
    }
    return NULL; // not found
}

void add_neighbor(RoomNode *room, RoomNode *neighbor) {
    for (int i = 0; i < MAX_NEIGHBORS; ++i) {
        if (room->neighbors[i] == NULL) {
            room->neighbors[i] = neighbor;
            return;
        }
    }
}

float compute_occupancy_rate(RoomNode *room) {
    if (room->room_capacity == 0)
        return 0.0; // Avoid division by zero
    return ((float)room->current_visitor_count / (float)room->room_capacity) * 100.0;
}

void establish_neighbors() {

    add_neighbor(&rooms[0], &rooms[1]); // Room 1 is neighbouring Room 2

    add_neighbor(&rooms[1], &rooms[0]); // Room 2 is neighbouring Room 1
    add_neighbor(&rooms[1], &rooms[2]); // and Room 3
    add_neighbor(&rooms[1], &rooms[3]); // and Room 4

    add_neighbor(&rooms[2], &rooms[1]); // Room 3 is neighbouring Room 2

    add_neighbor(&rooms[3], &rooms[1]); // Room 4 is neighbouring Room 3
}
