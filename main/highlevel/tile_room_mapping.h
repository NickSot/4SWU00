#ifndef TILE_ROOM_MAPPING_H
#define TILE_ROOM_MAPPING_H

#define MAX_NEIGHBORS 4
#define ROOM_ID_SIZE 6

#include "stdint.h"

typedef struct RoomNode {
    uint8_t roomID[ROOM_ID_SIZE]; // can be the same as the receiver's MAC address
    struct RoomNode *neighbors[MAX_NEIGHBORS];
} RoomNode;

void add_neighbor(RoomNode *room, RoomNode *neighbor);
void init_roomNode(RoomNode *room, uint8_t id[ROOM_ID_SIZE]);

#endif
