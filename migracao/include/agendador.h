#ifndef AGENDADOR_IDF_H
#define AGENDADOR_IDF_H

#include "estruturas.h"

/**
 * Inicializa os pinos dos relés usando o driver nativo do ESP-IDF.
 */
void inicializarRelesIdf(void);

/**
 * Tarefa do FreeRTOS para a avaliação do motor de agendamento (polling 1s).
 *
 * @param pvParameters Ponteiro para a struct ConfiguracaoIrrigacao.
 */
void tarefaAgendadorIdf(void *pvParameters);

/**
 * Auto-verificação imediata do pino do relé contra acionamentos espúrios por ruído elétrico.
 */
void verificarEProtegerEstadoReleFisicoIdf(void);

/**
 * Retorna o total de disparos espúrios debelados pelo firmware.
 */
uint32_t obterContadorDisparosEspuriosIdf(void);

/**
 * Executa controle manual do relé.
 */
void controleManualIdf(uint8_t canal, bool estado);

/**
 * Retorna o estado lógico do relé.
 */
bool obterEstadoReleIdf(uint8_t canal);

#endif // AGENDADOR_IDF_H
