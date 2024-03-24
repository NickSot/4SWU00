#include "aps/esp_zigbee_aps.h"
#include "data_structures/beacon_hashtable.h"
#include "esp_zigbee_core.h"
#include "string.h"

esp_err_t send_beacon_data(BeaconData *beacon_data_ptr);
esp_err_t receive_beacon_data(esp_zb_apsde_data_confirm_t *packet, BeaconHashTable *ht_ptr);