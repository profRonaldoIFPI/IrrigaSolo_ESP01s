/**
 * main.c — Ponto de Entrada da Migração IrrigaSolo em ESP8266 RTOS SDK
 *
 * Demonstra a arquitetura profissional nativa baseada no ESP8266_RTOS_SDK (FreeRTOS) no NodeMCU ESP-12E:
 *   1. Inicialização de NVS (Non-Volatile Storage)
 *   2. Segurança de hardware inicial (GPIO LOW imediato no boot)
 *   3. Criação e escalonamento de tarefas do FreeRTOS (xTaskCreate)
 *
 * Idioma: Português
 * Convenções: camelCase para funções/variáveis, SCREAMING_SNAKE_CASE para constantes.
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "pinosHardware.h"
#include "estruturas.h"
#include "agendador.h"
#include "autovigilancia.h"
#include "redeWifi.h"
#include "relogioNtp.h"
#include "sensorFluxo.h"
#include "clienteMqtt.h"

static const char *TAG = "MAIN_RTOS";

// Configurações globais do sistema
static ConfiguracaoRede configRede;
static ConfiguracaoIrrigacao configIrrigacao;

void app_main(void) {
    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "  IrrigaSolo ESP8266 RTOS SDK (%s) — Boot", VERSAO_FIRMWARE_IDF);
    ESP_LOGI(TAG, "========================================");

    // 1. Segurança de Hardware Imediata — GPIO 5 LOW no primeiro milissegundo
    inicializarRelesIdf();

    // 2. Inicialização da memória NVS Flash
    esp_err_t retornoNvs = nvs_flash_init();
    if (retornoNvs == ESP_ERR_NVS_NO_FREE_PAGES || retornoNvs == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        retornoNvs = nvs_flash_init();
    }
    ESP_ERROR_CHECK(retornoNvs);
    ESP_LOGI(TAG, "[NVS] Memória não-volátil inicializada com sucesso.");

    // 3. Inicializar módulo de autovigilância e diagnóstico de reset
    inicializarAutovigilanciaIdf();

    // 4. Criar Tarefa do Agendador no FreeRTOS (Prioridade 5, 2KB Stack)
    xTaskCreate(
        tarefaAgendadorIdf,
        "tarefaAgendador",
        2048,
        &configIrrigacao,
        5,
        NULL
    );
    ESP_LOGI(TAG, "[FreeRTOS] Tarefa do Agendador criada.");

    // 5. Criar Tarefa de Autovigilância no FreeRTOS (Prioridade 2, 2KB Stack)
    xTaskCreate(
        tarefaAutovigilanciaIdf,
        "tarefaAutovigilancia",
        2048,
        NULL,
        2,
        NULL
    );
    ESP_LOGI(TAG, "[FreeRTOS] Tarefa de Autovigilância criada.");

    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "  IrrigaSolo ESP8266 RTOS SDK pronto e operacional!");
    ESP_LOGI(TAG, "========================================");
}
