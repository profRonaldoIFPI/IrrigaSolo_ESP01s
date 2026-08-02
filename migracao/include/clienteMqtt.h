#ifndef CLIENTE_MQTT_IDF_H
#define CLIENTE_MQTT_IDF_H

#include "estruturas.h"

/**
 * Inicializa o cliente MQTT nativo do ESP-IDF (esp_mqtt_client).
 */
void inicializarMqttIdf(const ConfiguracaoRede* config, ConfiguracaoIrrigacao* configIrrigacao);

/**
 * Publica a telemetria periódica em JSON.
 */
void publicarTelemetriaIdf(bool valvulaLigada, float vazaoLMin, float volumeTotalL, const char* ultimaIrrigacao, const char* proximaIrrigacao);

/**
 * Publica o anúncio de reboot contendo o motivo do reset e estatísticas no tópico dedicated.
 */
void publicarStatusRebootIdf(void);

/**
 * Publica as 6 sessões de agendamento cadastradas no tópico dedicado.
 */
void publicarAgendamentosIdf(void);

/**
 * Retorna se o cliente MQTT está conectado ao Broker.
 */
bool obterEstadoConexaoMqttIdf(void);

#endif // CLIENTE_MQTT_IDF_H
