#pragma once

/**
 * pinosHardware.h — Definições de Pinos e Constantes de Hardware (ESP-01S)
 *
 * Este header centraliza todas as constantes de pinagem e parâmetros operacionais
 * do sistema IrrigaSolo adaptado para o microcontrolador ESP8266 ESP-01S.
 * 
 * Contexto de Hardware do ESP-01S:
 * O ESP-01S expõe apenas 8 pinos físicos: VCC, GND, RST, CH_PD (EN), TX (GPIO1),
 * RX (GPIO3), GPIO0 e GPIO2.
 * - GPIO0: Linha conectada ao chaveamento do relé (módulo relé v4.0 para ESP-01S).
 * - GPIO2: Conectado ao LED onboard azul do ESP-01S (lógica invertida).
 * - Sensor de fluxo: Descontinuado nesta variante por falta de pinos livres (ADR-014).
 *
 * Convenção de nomenclatura: SCREAMING_SNAKE_CASE para constantes e macros.
 */

#include <Arduino.h>

// ---------------------------------------------------------------------------
// Pinos de Controle Físico (ADR-007 e ADR-014)
// ---------------------------------------------------------------------------

/**
 * Pino de controle do relé da válvula de irrigação.
 * No módulo relé v4.0 para ESP-01S, o relé é acionado pelo GPIO 0
 * através de um transistor NPN (lógica Active HIGH).
 */
static const uint8_t PINO_RELE_CANAL_1 = 0; // GPIO0

/**
 * Níveis lógicos de acionamento do relé (Active HIGH no módulo relé v4.0).
 */
static const uint8_t ESTADO_RELE_LIGADO    = HIGH;
static const uint8_t ESTADO_RELE_DESLIGADO = LOW;

/**
 * Pino do LED onboard do ESP-01S (GPIO2).
 * Utilizado como indicador visual de status da conexão Wi-Fi:
 *   - Piscando = Modo AP (aguardando configuração ou reconexão)
 *   - Fixo (ligado) = Conectado à rede STA local
 * Nota: O LED onboard do ESP-01S opera em lógica invertida (LOW = aceso).
 */
static const uint8_t PINO_LED_STATUS = 2; // GPIO2

/**
 * Níveis lógicos do LED onboard do ESP-01S (Active LOW).
 */
static const uint8_t ESTADO_LED_LIGADO    = LOW;
static const uint8_t ESTADO_LED_DESLIGADO = HIGH;

// ---------------------------------------------------------------------------
// Número de Canais de Irrigação
// ---------------------------------------------------------------------------
static const uint8_t TOTAL_CANAIS = 1;

// ---------------------------------------------------------------------------
// Limites de Agendamento (RF1.1: até 6 sessões por canal por dia)
// ---------------------------------------------------------------------------
static const uint8_t MAX_SESSOES_POR_CANAL = 6;

// ---------------------------------------------------------------------------
// Configurações Padrão do Access Point (RF2.1 / ADR-004)
// Utilizadas quando o arquivo /network.json está ausente ou corrompido.
// ---------------------------------------------------------------------------
static const char SSID_AP_PADRAO[]  = "IrrigaSolo";
static const char SENHA_AP_PADRAO[] = "admin123";

// ---------------------------------------------------------------------------
// Configurações Padrão do NTP (ADR-004)
// ---------------------------------------------------------------------------
static const char   SERVIDOR_NTP_PADRAO[] = "pool.ntp.br";
static const int8_t FUSO_HORARIO_PADRAO   = -3;

/**
 * Intervalo padrão de sincronização NTP em segundos.
 * 21600s = 6 horas, conforme o esquema network.json do ADR-004.
 */
static const uint32_t INTERVALO_NTP_PADRAO = 21600;

// ---------------------------------------------------------------------------
// Intervalo do Scheduler (ADR-005: Super Loop polling)
// Granularidade de 1 segundo para avaliar o minuto exato de ativação.
// ---------------------------------------------------------------------------
static const uint32_t INTERVALO_AVALIACAO_MS = 1000;

// ---------------------------------------------------------------------------
// Timeout de Conexão Wi-Fi STA (em milissegundos)
// Aproximadamente 15 segundos com yield() a cada iteração.
// ---------------------------------------------------------------------------
static const uint32_t TIMEOUT_CONEXAO_STA_MS = 15000;

// ---------------------------------------------------------------------------
// Intervalo do LED piscante em Modo AP (milissegundos)
// ---------------------------------------------------------------------------
static const uint32_t INTERVALO_LED_AP_MS = 500;

// ---------------------------------------------------------------------------
// Atraso de Desacoplamento Assíncrono do Relé (ADR-015)
// Janela de espera não-bloqueante (em milissegundos) entre o recebimento de
// comandos de rede (MQTT/HTTP) e o chaveamento físico no GPIO 0, permitindo
// que as transmissões RF do Wi-Fi finalizem antes do choque indutivo do relé.
// ---------------------------------------------------------------------------
static const uint32_t ATRASO_DESACOPLAMENTO_RELE_MS = 150;

// ---------------------------------------------------------------------------
// Velocidade da porta Serial para telemetria (ADR-008)
// ---------------------------------------------------------------------------
static const uint32_t VELOCIDADE_SERIAL = 115200;

// ---------------------------------------------------------------------------
// Versão do Firmware IrrigaSolo para ESP-01S
// ---------------------------------------------------------------------------
static const char VERSAO_FIRMWARE[] = "v2.0-esp01s";
