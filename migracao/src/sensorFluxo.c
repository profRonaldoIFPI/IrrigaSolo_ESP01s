/**
 * sensorFluxo.c — Driver do Sensor de Fluxo com Interrupção de Hardware em ESP-IDF
 *
 * Utiliza o driver gpio_isr_handler_add para contar pulsos de vazão no ESP32.
 */

#include "sensorFluxo.h"
#include "pinosHardware.h"
#include "esp_log.h"
#include "driver/gpio.h"

static const char *TAG = "SENSOR_FLUXO_IDF";

static volatile uint32_t contadorPulsos = 0;
static float vazaoAtualLMin = 0.0f;
static float volumeTotalAcumuladoL = 0.0f;

// ISR (Interrupt Service Routine) atribuída à GPIO IRAM para performance máxima
static void IRAM_ATTR handlerInterrupcaoFluxo(void* arg) {
    contadorPulsos++;
}

void inicializarSensorFluxoIdf(void) {
    gpio_config_t ioConfig = {
        .pin_bit_mask = (1ULL << PINO_SENSOR_FLUXO),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_NEGEDGE
    };
    gpio_config(&ioConfig);

    // Instala serviço de interrupção ISR no ESP-IDF
    gpio_install_isr_service(0);
    gpio_isr_handler_add(PINO_SENSOR_FLUXO, handlerInterrupcaoFluxo, NULL);

    ESP_LOGI(TAG, "[ISR] Interrupção do sensor de fluxo anexada ao pino GPIO %d.", PINO_SENSOR_FLUXO);
}

void processarSensorFluxoIdf(void) {
    uint32_t pulsos = contadorPulsos;
    contadorPulsos = 0;

    vazaoAtualLMin = (float)pulsos / FATOR_CALIBRACAO_FLUXO;
    volumeTotalAcumuladoL += (vazaoAtualLMin / 60.0f);
}

float obterVazaoLMinIdf(void) {
    return vazaoAtualLMin;
}

float obterVolumeTotalLIdf(void) {
    return volumeTotalAcumuladoL;
}

void zerarVolumeTotalIdf(void) {
    volumeTotalAcumuladoL = 0.0f;
    ESP_LOGI(TAG, "[SENSOR] Volume de água acumulado zerado.");
}
