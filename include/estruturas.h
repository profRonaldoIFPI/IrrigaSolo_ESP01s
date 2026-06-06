#pragma once

/**
 * estruturas.h — Estruturas de Dados do Sistema IrrigaSolo
 *
 * Define as structs que representam os dados de configuração do sistema,
 * mapeando diretamente os esquemas JSON persistidos no LittleFS (ADR-004).
 *
 * Estas estruturas são a representação em memória RAM dos arquivos
 * /network.json e /irrigation.json. A separação em dois domínios
 * (rede e irrigação) segue a decisão arquitetural de isolamento
 * para reduzir desgaste de Flash e risco de corrupção cruzada.
 *
 * Convenção: PascalCase para structs, camelCase para membros.
 */

#include "pinosHardware.h"

// ---------------------------------------------------------------------------
// SessaoIrrigacao — Representa uma sessão de irrigação individual (RF1.1)
// ---------------------------------------------------------------------------

/**
 * Cada canal suporta até MAX_SESSOES_POR_CANAL (6) sessões por dia.
 * Uma sessão define: se está habilitada, o horário de início (HH:MM)
 * e a duração em minutos.
 *
 * O campo 'inicio' armazena a string "HH:MM" (5 chars + null terminator).
 * O campo 'duracao' usa uint8_t (0–255 min), suficiente para o limite
 * funcional de irrigação.
 */
struct SessaoIrrigacao {
    bool    habilitada;
    char    inicio[6];   // Formato "HH:MM\0"
    uint8_t duracao;     // Duração em minutos
};

// ---------------------------------------------------------------------------
// ConfiguracaoRede — Esquema do /network.json (ADR-004)
// ---------------------------------------------------------------------------

/**
 * Representa todas as configurações de rede e NTP persistidas no LittleFS.
 * Os tamanhos dos buffers foram dimensionados para os limites práticos
 * do ESP8266 (SSID: 32 chars, senha WPA2: 63 chars, URL NTP: 64 chars).
 */
struct ConfiguracaoRede {
    char     ssidAp[33];         // SSID do Access Point
    char     senhaAp[64];        // Senha do Access Point
    char     ssidSta[33];        // SSID da rede local (estação)
    char     senhaSta[64];       // Senha da rede local
    char     servidorNtp[65];    // URL do servidor NTP
    int8_t   fusoHorario;        // Offset UTC (ex: -3 para Brasília)
    uint32_t intervaloSyncNtp;   // Intervalo de sincronização em segundos
};

// ---------------------------------------------------------------------------
// ConfiguracaoIrrigacao — Esquema do /irrigation.json (ADR-004)
// ---------------------------------------------------------------------------

/**
 * Agrupa a configuração completa de irrigação: a flag de simultaneidade
 * (RF3) e os arrays de sessões dos dois canais.
 *
 * Quando 'permitirSimultaneidade' é false, o firmware impede que horários
 * de Canal 1 e Canal 2 se sobreponham (ADR-006, RF3.1).
 */
struct ConfiguracaoIrrigacao {
    bool             permitirSimultaneidade;
    SessaoIrrigacao  canal1[MAX_SESSOES_POR_CANAL];
    SessaoIrrigacao  canal2[MAX_SESSOES_POR_CANAL];
};
