#ifndef AUTOVIGILANCIA_IDF_H
#define AUTOVIGILANCIA_IDF_H

#include <stdint.h>

/**
 * Inicializa a captura da razão de reset do ESP32 via esp_reset_reason().
 */
void inicializarAutovigilanciaIdf(void);

/**
 * Tarefa do FreeRTOS para monitoramento contínuo de Heap e resiliência de rede.
 */
void tarefaAutovigilanciaIdf(void *pvParameters);

/**
 * Retorna a string com o motivo do último reset.
 */
const char* obterMotivoResetIdf(void);

#endif // AUTOVIGILANCIA_IDF_H
