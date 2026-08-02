#ifndef REDE_WIFI_IDF_H
#define REDE_WIFI_IDF_H

#include "estruturas.h"

/**
 * Inicializa o Wi-Fi no ESP-IDF utilizando esp_netif e esp_event.
 */
void inicializarWifiIdf(const ConfiguracaoRede* config);

/**
 * Retorna true se a estação Wi-Fi estiver conectada à rede.
 */
bool estaConectadoStaIdf(void);

/**
 * Retorna o IP atual em formato string.
 */
const char* obterIpAtualIdf(void);

#endif // REDE_WIFI_IDF_H
