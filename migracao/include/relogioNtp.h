#ifndef RELOGIO_NTP_IDF_H
#define RELOGIO_NTP_IDF_H

#include "estruturas.h"

/**
 * Inicializa o serviço SNTP nativo do ESP-IDF (esp_sntp).
 */
void inicializarNtpIdf(const ConfiguracaoRede* config);

/**
 * Retorna os minutos decorridos desde a meia-noite (0–1439).
 */
int obterMinutosDesdeMeiaNoiteIdf(void);

/**
 * Retorna a hora formatada em string "HH:MM:SS".
 */
const char* obterHoraFormatadaIdf(void);

#endif // RELOGIO_NTP_IDF_H
