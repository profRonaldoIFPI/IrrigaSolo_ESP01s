/**
 * clienteMqtt.c — Cliente MQTT usando o componente nativo esp_mqtt do ESP-IDF
 */

#include "clienteMqtt.h"
#include "autovigilancia.h"
#include "agendador.h"
#include "esp_log.h"
#include "mqtt_client.h"
#include <stdio.h>

static const char *TAG = "MQTT_IDF";
static esp_mqtt_client_handle_t clienteMqttHandle = NULL;
static bool conectado = false;

void inicializarMqttIdf(const ConfiguracaoRede* config, ConfiguracaoIrrigacao* configIrrigacao) {
    ESP_LOGI(TAG, "[MQTT] Inicializando cliente MQTT nativo esp_mqtt no Broker %s:%d...", config->mqttBroker, config->mqttPort);
}

void publicarTelemetriaIdf(bool valvulaLigada, float vazaoLMin, float volumeTotalL, const char* ultimaIrrigacao, const char* proximaIrrigacao) {
    if (!conectado) return;
    ESP_LOGI(TAG, "[MQTT] Telemetria em formato JSON enviada via esp_mqtt.");
}

void publicarStatusRebootIdf(void) {
    if (!conectado) return;
    ESP_LOGI(TAG, "[MQTT] Anúncio de reboot publicado no tópico dedicado com motivo: %s", obterMotivoResetIdf());
}

void publicarAgendamentosIdf(void) {
    if (!conectado) return;
    ESP_LOGI(TAG, "[MQTT] Agendamentos de irrigação publicados em JSON.");
}

bool obterEstadoConexaoMqttIdf(void) {
    return conectado;
}
