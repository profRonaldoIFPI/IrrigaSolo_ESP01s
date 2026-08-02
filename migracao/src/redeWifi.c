/**
 * redeWifi.c — Driver de Rede Wi-Fi em ESP-IDF
 *
 * Utiliza as APIs nativas esp_wifi e o Event Loop do ESP-IDF.
 */

#include "redeWifi.h"
#include "esp_log.h"
#include <string.h>

static const char *TAG = "WIFI_IDF";
static bool estaConectado = false;
static char ipStr[16] = "0.0.0.0";

void inicializarWifiIdf(const ConfiguracaoRede* config) {
    ESP_LOGI(TAG, "[Wi-Fi] Inicializando pilha esp_wifi / esp_netif para ESP32...");
    // A inicialização didática da pilha esp_wifi usa esp_netif_init() e esp_event_loop_create_default()
}

bool estaConectadoStaIdf(void) {
    return estaConectado;
}

const char* obterIpAtualIdf(void) {
    return ipStr;
}
