#pragma once

/**
 * redeWifi.h — Interface do Módulo de Gerenciamento Wi-Fi
 *
 * Controla a conexão Wi-Fi do ESP8266 nos modos STA (estação) e AP
 * (Access Point). Implementa o fallback automático para AP quando a
 * rede local não está disponível (RF2.3), sem interromper os
 * agendamentos já carregados em memória.
 *
 * Decisão arquitetural chave (ADR-003): o cache nativo do SDK para
 * credenciais Wi-Fi é desabilitado via WiFi.persistent(false).
 * A única fonte de verdade são os dados do LittleFS.
 *
 * Requisitos atendidos: RF2.1, RF2.2, RF2.3, RF5.1, ADR-003.
 */

#include "estruturas.h"

/**
 * Inicializa o subsistema Wi-Fi do ESP8266.
 *
 * Desabilita WiFi.persistent(false) para garantir que o SDK não
 * armazene credenciais na Flash interna (ADR-003). Tenta conectar
 * em modo STA se houver SSID configurado; caso contrário, inicia
 * diretamente em modo AP com as credenciais da struct.
 *
 * @param configRede Referência constante com as credenciais de rede.
 */
void inicializarWifi(const ConfiguracaoRede& configRede);

/**
 * Verifica periodicamente o estado da conexão Wi-Fi.
 *
 * Deve ser chamada no loop() principal. Se a conexão STA for perdida,
 * ativa automaticamente o modo AP para permitir reconfiguração (RF2.3),
 * mantendo os agendamentos de irrigação intactos na memória.
 *
 * Também gerencia o LED de status (piscando = AP, fixo = STA).
 */
void verificarConexaoWifi();

/**
 * Retorna true se o ESP8266 está conectado à rede local (modo STA).
 * Função inline para eliminar overhead de chamada no loop().
 */
inline bool estaConectadoSta();

/**
 * Retorna o endereço IP atual do dispositivo.
 * Em modo STA, retorna o IP atribuído pelo roteador.
 * Em modo AP, retorna o IP do gateway (tipicamente 192.168.4.1).
 *
 * @return String com o IP no formato "X.X.X.X".
 */
String obterIpAtual();
