/**
 * autovigilancia.c — Módulo de Autovigilância ESP-IDF / FreeRTOS
 *
 * Utiliza as APIs nativas esp_reset_reason() e esp_get_free_heap_size().
 */

#include "autovigilancia.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_system.h"
#include <string.h>

static const char *TAG = "AUTOVIGILANCIA_IDF";
static char motivoResetStr[64] = "Desconhecido";

void inicializarAutovigilanciaIdf(void) {
    esp_reset_reason_t razao = esp_reset_reason();

    switch (razao) {
        case ESP_RST_POWERON:
            snprintf(motivoResetStr, sizeof(motivoResetStr), "Power-on Reset (Energizado)");
            break;
        case ESP_RST_SW:
            snprintf(motivoResetStr, sizeof(motivoResetStr), "Software Reset (ESP.restart)");
            break;
        case ESP_RST_PANIC:
            snprintf(motivoResetStr, sizeof(motivoResetStr), "Exception / Panic Reset (Crash)");
            break;
        case ESP_RST_INT_WDT:
            snprintf(motivoResetStr, sizeof(motivoResetStr), "Interrupt Watchdog Reset");
            break;
        case ESP_RST_TASK_WDT:
            snprintf(motivoResetStr, sizeof(motivoResetStr), "Task Watchdog Reset");
            break;
        case ESP_RST_BROWNOUT:
            snprintf(motivoResetStr, sizeof(motivoResetStr), "Brownout Reset (Queda de Tensão)");
            break;
        default:
            snprintf(motivoResetStr, sizeof(motivoResetStr), "Reset Outro (%d)", razao);
            break;
    }

    ESP_LOGI(TAG, "[RESET] Razão de inicialização detectada: %s", motivoResetStr);
}

const char* obterMotivoResetIdf(void) {
    return motivoResetStr;
}

void tarefaAutovigilanciaIdf(void *pvParameters) {
    ESP_LOGI(TAG, "[FreeRTOS] Tarefa de Autovigilância iniciada.");

    while (1) {
        // Monitora o Heap livre a cada 10 segundos
        uint32_t heapLivre = esp_get_free_heap_size();
        ESP_LOGI(TAG, "[WATCHDOG] Heap livre atual: %u bytes", heapLivre);

        // Se o Heap estiver abaixo de 8 KB de forma crítica
        if (heapLivre < 8192) {
            ESP_LOGW(TAG, "[WATCHDOG] Nível crítico de memória Heap! (%u bytes)", heapLivre);
        }

        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}
