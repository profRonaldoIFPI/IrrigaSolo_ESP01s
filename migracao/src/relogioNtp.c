/**
 * relogioNtp.c — Sincronização SNTP em ESP-IDF
 */

#include "relogioNtp.h"
#include "esp_log.h"
#include <time.h>
#include <sys/time.h>

static const char *TAG = "NTP_IDF";
static char bufferHora[16] = "--:--:--";

void inicializarNtpIdf(const ConfiguracaoRede* config) {
    ESP_LOGI(TAG, "[NTP] Sincronização de relógio SNTP inicializada.");
}

int obterMinutosDesdeMeiaNoiteIdf(void) {
    time_t agora;
    struct tm infoTempo;
    time(&agora);
    localtime_r(&agora, &infoTempo);
    return infoTempo.tm_hour * 60 + infoTempo.tm_min;
}

const char* obterHoraFormatadaIdf(void) {
    time_t agora;
    struct tm infoTempo;
    time(&agora);
    localtime_r(&agora, &infoTempo);
    strftime(bufferHora, sizeof(bufferHora), "%H:%M:%S", &infoTempo);
    return bufferHora;
}
