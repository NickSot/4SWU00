#include "esp_app_trace.h"
#include "esp_log.h"
#include "esp_zigbee_core.h"

#include "data_structures/beacon_hashtable.h"

void tile_main();
void monitor_main();

void app_main(void) {
#if defined(ZB_COORDINATOR_ROLE) // COORDINATOR ROLE
    ESP_LOGI("COMPASS_INIT", "Functioning as monitor");

    monitor_main();
#elif defined(ZB_ED_ROLE) // End device role
    ESP_LOGI("COMPASS_INIT", "Functioning as tile");

    tile_main();
#else
#error one of CZB_COORDINATOR_ROLE or ZB_ED_ROLE must be defined to configure the hardware type
#endif
}
