#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "tile_room_mapping.h"

void add_neighbor(RoomNode *room, RoomNode *neighbor) {
    for (int i = 0; i < MAX_NEIGHBORS; ++i) {
        if (room->neighbors[i] == NULL) {
            room->neighbors[i] = neighbor;
            return;
        }
    }
}

void init_roomNode(RoomNode *room, char *id) {
    strcpy(room->roomID, id);
    for (int i = 0; i < MAX_NEIGHBORS; ++i) {
        room->neighbors[i] = NULL;
    }
}
