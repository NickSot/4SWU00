idf_component_register(
    SRCS
    "esp_zb_coordinator.c"
    "network_flow_driver.c"
    "data_structs/beacon_hashtable.c"
    "data_structs/tile_room_mapping.c"
    INCLUDE_DIRS "." "./data_structs"
)
