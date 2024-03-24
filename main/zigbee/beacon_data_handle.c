#include "beacon_data_handle.h"
#include "data_structures/beacon_hashtable.h"

static const char *TAG = "BEACON_DATA_HANDLER";

esp_err_t send_beacon_data(BeaconData *beacon_data_ptr) {
    esp_zb_apsde_data_req_t cmd_req;

    cmd_req.dst_addr_mode = ESP_ZB_APS_ADDR_MODE_16_ENDP_PRESENT;
    cmd_req.dst_endpoint = 0xff;
    cmd_req.asdu_length = sizeof(BeaconData);
    cmd_req.asdu = (uint8_t *)beacon_data_ptr;
    cmd_req.cluster_id = 1;
    cmd_req.profile_id = 1;

    cmd_req.dst_short_addr = 0xffff;
    cmd_req.use_alias = false;
    cmd_req.radius = 0x5;
    cmd_req.src_endpoint = esp_zb_get_short_address();
    cmd_req.tx_options = ESP_ZB_APSDE_TX_OPT_NO_LONG_ADDR;

    ESP_EARLY_LOGI(TAG, "Send serialized composite data structure");

    return esp_zb_aps_data_request(&cmd_req);
}

esp_err_t receive_beacon_data(esp_zb_apsde_data_confirm_t *packet, BeaconHashTable *ht_ptr) {
    if (packet->asdu_length != sizeof(BeaconData)) {
        ESP_LOGW(TAG, "Ignoring data of length %ld", packet->asdu_length);
        return ESP_FAIL;
    }

    BeaconData beacon_data;
    memcpy(&beacon_data, packet->asdu, sizeof(BeaconData));

    ESP_LOGI(TAG, BEACONSTR, BEACON2STR(beacon_data));

    BeaconUpdateStatus status = insert_update_beacon(ht_ptr, beacon_data);

    if (status == BEACON_DROPPED) {
        ESP_LOGW(TAG, "New beacon was dropped");
        return ESP_FAIL;
    }

    return ESP_OK;
}