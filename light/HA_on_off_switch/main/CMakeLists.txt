idf_component_register(
    SRCS
    "esp_zb_switch.c"
    "switch_driver.c"
    "data_structs/beacon_hashtable.c"
    "data_structs/tile_room_mapping.c"
    INCLUDE_DIRS "." "./data_structs"
)
