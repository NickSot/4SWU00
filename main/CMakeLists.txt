idf_component_register(
    SRCS
        "main.c"
        # "esp_zb_coordinator.c"
        # "network_flow_driver.c"

        "eddystone/esp_eddystone_api.c"
        "eddystone/eddystone_impl.c"

        "data_structures/beacon_hashtable.c"
        "data_structures/tile_room_mapping.c"
    INCLUDE_DIRS
        "."
        "eddystone"
        "data_structures"
)
