#pragma once

/**
 * pinosHardware.h — Definições de Pinos e Constantes de Hardware
 *
 * Este header centraliza todas as constantes relacionadas ao hardware físico
 * do sistema IrrigaSolo. Alterar um pino GPIO ou um valor padrão aqui reflete
 * automaticamente em todo o firmware, evitando "números mágicos" espalhados
 * pelo código.
 *
 * Convenção de nomenclatura: SCREAMING_SNAKE_CASE para constantes e macros.
 */

#include <Arduino.h>

// ---------------------------------------------------------------------------
// Pinos dos Relés (ADR-007: devem ser configurados como OUTPUT e LOW
// imediatamente no início do setup(), antes de qualquer outra inicialização)
// ---------------------------------------------------------------------------

/**
 * Canal 1 conectado ao pino D1 (GPIO5) do NodeMCU.
 * Escolhido por ser um pino seguro: não afeta o boot do ESP8266
 * e não possui funções especiais de inicialização.
 */
static const uint8_t PINO_RELE_CANAL_1 = D1; // GPIO5

/**
 * Canal 2 conectado ao pino D2 (GPIO4) do NodeMCU.
 * Mesma justificativa de segurança do Canal 1.
 */
static const uint8_t PINO_RELE_CANAL_2 = D2; // GPIO4

/**
 * LED onboard do NodeMCU (D4 / GPIO2).
 * Utilizado como indicador visual de status da conexão Wi-Fi:
 *   - Piscando = Modo AP (aguardando configuração)
 *   - Fixo (ligado) = Conectado à rede STA
 * Nota: O LED onboard do ESP8266 opera em lógica invertida (LOW = aceso).
 */
static const uint8_t PINO_LED_STATUS = D4; // GPIO2

// ---------------------------------------------------------------------------
// Número de Canais de Irrigação
// ---------------------------------------------------------------------------
static const uint8_t TOTAL_CANAIS = 2;

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
// Granularidade de 1 segundo para ativar no minuto exato (RF1.1).
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
// Velocidade da porta Serial para telemetria (ADR-008)
// ---------------------------------------------------------------------------
static const uint32_t VELOCIDADE_SERIAL = 115200;
