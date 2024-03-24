idf_component_register(
    SRCS
        "main.c"
        "monitor.c"
        "tile.c"

        "eddystone/esp_eddystone_api.c"
        "eddystone/eddystone_impl.c"

        # "zigbee/esp_zb_end_device.c"
        # "zigbee/esp_zb_coordinator.c"
        "zigbee/beacon_data_handle.c"

        "data_structures/beacon_hashtable.c"
        "data_structures/tile_room_mapping.c"
    INCLUDE_DIRS
        "."
        "eddystone"
        "zigbee"
        "data_structures"
)
