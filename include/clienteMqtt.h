#pragma once

/**
 * clienteMqtt.h — Interface do Cliente MQTT para Telemetria e Comandos
 *
 * Gerencia a conexão com o broker MQTT (não-bloqueante), escuta comandos
 * de acionamento remoto da válvula e publica dados de telemetria em formato JSON.
 *
 * Idioma: Português
 * Convenção: camelCase para funções e variáveis.
 */

#include "estruturas.h"
#include "autovigilancia.h"

/**
 * Inicializa a biblioteca PubSubClient com as configurações de rede/broker carregadas
 * e define a função de callback para recebimento de comandos.
 *
 * @param config Referência para a struct de configuração de rede contendo os dados do Broker.
 * @param configIrrigacao Ponteiro opcional para a struct de agendamentos de irrigação.
 */
void inicializarMqtt(const ConfiguracaoRede& config, const ConfiguracaoIrrigacao* configIrrigacao = nullptr);

/**
 * Registra o ponteiro da configuração de agendamentos de irrigação no módulo MQTT.
 *
 * @param config Ponteiro para a estrutura de agendamentos de irrigação.
 */
void registrarConfiguracaoIrrigacaoMqtt(const ConfiguracaoIrrigacao* config);

/**
 * Mantém a conexão ativa com o Broker MQTT e processa a fila de mensagens recebidas.
 * Deve ser chamada em todas as iterações do loop() principal.
 * Se houver desconexão, tenta reconectar sem travar o processamento local (não-bloqueante).
 */
void processarMqtt();

/**
 * Publica os dados de telemetria instantâneos no broker MQTT.
 *
 * @param valvulaLigada true se a válvula está ativa (relé HIGH), false caso contrário.
 * @param vazaoLMin Vazão instantânea medida em litros/minuto.
 * @param volumeTotalL Volume acumulado de água em litros.
 * @param ultimaIrrigacao Horário de início da última rega ("HH:MM").
 * @param proximaIrrigacao Horário de início da próxima rega ("HH:MM").
 */
void publicarTelemetria(bool valvulaLigada, float vazaoLMin, float volumeTotalL, const char* ultimaIrrigacao, const char* proximaIrrigacao);

/**
 * Publica o anúncio de reboot contendo motivo do reset, uptime e disparos espúrios
 * no tópico dedicated `<mqttPublishTopic>/reboot` (RF5.3, CA-09).
 */
void publicarStatusReboot();

/**
 * Publica as 6 sessões de agendamento de irrigação em formato JSON
 * no tópico dedicado `<mqttPublishTopic>/agendamentos` (RF5.3, CA-09).
 */
void publicarAgendamentos();

/**
 * Retorna se o cliente está conectado ao Broker MQTT.
 *
 * @return true se conectado, false caso contrário.
 */
bool obterEstadoConexaoMqtt();
