#ifndef TILE_ROOM_MAPPING_H
#define TILE_ROOM_MAPPING_H

#define MAX_NEIGHBORS 4

typedef struct RoomNode {
    char roomID[18]; // can be the same as the receiver's MAC address
    struct RoomNode *neighbors[MAX_NEIGHBORS];
} RoomNode;

void add_neighbor(RoomNode *room, RoomNode *neighbor);
void init_roomNode(RoomNode *room, char *id);

#endif
