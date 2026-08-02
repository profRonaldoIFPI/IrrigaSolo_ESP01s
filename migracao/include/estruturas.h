#ifndef ESTRUTURAS_IDF_H
#define ESTRUTURAS_IDF_H

#include "pinosHardware.h"

/**
 * Representa uma sessão de irrigação individual (RF1.1).
 */
typedef struct {
    bool    habilitada;
    char    inicio[6];   // Formato "HH:MM\0"
    uint8_t duracao;     // Duração em minutos
} SessaoIrrigacao;

/**
 * Configuração completa de irrigação.
 */
typedef struct {
    SessaoIrrigacao canal1[MAX_SESSOES_POR_CANAL];
} ConfiguracaoIrrigacao;

/**
 * Configuração de rede e parâmetros MQTT.
 */
typedef struct {
    char     ssidAp[33];
    char     senhaAp[64];
    char     ssidSta[33];
    char     senhaSta[64];
    char     servidorNtp[65];
    int8_t   fusoHorario;
    uint32_t intervaloSyncNtp;

    char     mqttBroker[65];
    uint16_t mqttPort;
    char     mqttUser[33];
    char     mqttPassword[65];
    char     mqttClientId[33];
    char     mqttPublishTopic[65];
    char     mqttSubscribeTopic[65];
    uint16_t mqttInterval;
} ConfiguracaoRede;

#endif // ESTRUTURAS_IDF_H
