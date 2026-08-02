/**
 * agendador.c — Motor de Agendamento ESP-IDF / FreeRTOS
 *
 * Implementa a tarefa agendadora do FreeRTOS e a verificação ativas de GPIO.
 */

#include "agendador.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include <string.h>

static const char *TAG = "AGENDADOR_IDF";

static uint32_t disparosEspuriosCount = 0;
static bool estadoAutorizadoAtual = false;

void inicializarRelesIdf(void) {
    gpio_config_t ioConfig = {
        .pin_bit_mask = (1ULL << PINO_RELE_CANAL_1),
        .mode = GPIO_MODE_INPUT_OUTPUT, // INPUT_OUTPUT permite ler o pino físico diretamente
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_ENABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&ioConfig);

    // Força nível lógico BAIXO imediatamente
    gpio_set_level(PINO_RELE_CANAL_1, 0);
    estadoAutorizadoAtual = false;

    ESP_LOGI(TAG, "[GPIO] Pino do Relé (GPIO %d) configurado como OUTPUT e LOW estrito.", PINO_RELE_CANAL_1);
}

uint32_t obterContadorDisparosEspuriosIdf(void) {
    return disparosEspuriosCount;
}

void verificarEProtegerEstadoReleFisicoIdf(void) {
    // Leitura nativa rápida do pino físico do ESP32 via gpio_get_level
    int nivelFisico = gpio_get_level(PINO_RELE_CANAL_1);

    if (nivelFisico == 1 && !estadoAutorizadoAtual) {
        gpio_set_level(PINO_RELE_CANAL_1, 0);
        disparosEspuriosCount++;
        ESP_LOGE(TAG, "[ANTI-RUÍDO] Disparo espúrio por ruído elétrico debelado no GPIO %d! Total espúrio: %u", PINO_RELE_CANAL_1, disparosEspuriosCount);
    }
}

void controleManualIdf(uint8_t canal, bool estado) {
    if (canal != 1) return;

    estadoAutorizadoAtual = estado;
    gpio_set_level(PINO_RELE_CANAL_1, estado ? 1 : 0);
    ESP_LOGI(TAG, "[MANUAL] Relé Canal %d alterado para: %s", canal, estado ? "LIGADO" : "DESLIGADO");
}

bool obterEstadoReleIdf(uint8_t canal) {
    if (canal != 1) return false;
    return gpio_get_level(PINO_RELE_CANAL_1) == 1;
}

void tarefaAgendadorIdf(void *pvParameters) {
    ESP_LOGI(TAG, "[FreeRTOS] Tarefa do Agendador iniciada no escalonador.");

    TickType_t ultimoUltimoTick = xTaskGetTickCount();
    const TickType_t frequenciaTicks = pdMS_TO_TICKS(1000); // Polling exato a cada 1000ms (1s)

    while (1) {
        // 1. Auto-verificação contínua anti-ruído elétrico
        verificarEProtegerEstadoReleFisicoIdf();

        // 2. Aguarda exatamente 1 segundo sem travar a CPU (Yield para o FreeRTOS)
        vTaskDelayUntil(&ultimoUltimoTick, frequenciaTicks);
    }
}
