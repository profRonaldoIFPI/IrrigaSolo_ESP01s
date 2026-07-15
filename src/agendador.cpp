/**
 * agendador.cpp — Implementação do Motor de Agendamento
 *
 * Lógica central do Super Loop de irrigação (ADR-005).
 *
 * Conceito de "janela de rega":
 *   Uma sessão com início "06:00" e duração 10 minutos cria uma
 *   janela de minutos [360, 370). Se o minuto atual do dia (obtido
 *   via obterMinutosDesdeMeiaNoite()) estiver dentro dessa janela,
 *   o relé correspondente é acionado. Caso contrário, é desligado.
 *
 * Canal Único:
 *   Controla apenas o Canal 1 de irrigação. As lógicas de conflito e
 *   simultaneidade foram removidas.
 */

#include "agendador.h"
#include <Arduino.h>

// ---------------------------------------------------------------------------
// Funções auxiliares internas
// ---------------------------------------------------------------------------

/**
 * Converte uma string "HH:MM" para minutos desde meia-noite.
 * Ex: "06:30" → 390.
 *
 * @param horario String no formato "HH:MM" (5 caracteres).
 * @return Minutos desde meia-noite (0–1439), ou -1 se formato inválido.
 */
static int converterHorarioParaMinutos(const char* horario) {
    // Validar formato mínimo: "HH:MM" = 5 caracteres com ':' na posição 2
    if (strlen(horario) < 5 || horario[2] != ':') {
        return -1;
    }

    int horas   = (horario[0] - '0') * 10 + (horario[1] - '0');
    int minutos = (horario[3] - '0') * 10 + (horario[4] - '0');

    // Validar limites
    if (horas < 0 || horas > 23 || minutos < 0 || minutos > 59) {
        return -1;
    }

    return horas * 60 + minutos;
}

/**
 * Retorna o pino GPIO correspondente ao canal informado.
 *
 * @param canal Número do canal (1 ou 2).
 * @return Pino GPIO, ou 0xFF se canal inválido.
 */
static uint8_t obterPinoDoCanal(uint8_t canal) {
    switch (canal) {
        case 1: return PINO_RELE_CANAL_1;
        default: return 0xFF;
    }
}

// ---------------------------------------------------------------------------
// Implementações públicas
// ---------------------------------------------------------------------------

void inicializarReles() {
    // ADR-007: Esta é a primeira ação do setup().
    // Configurar como OUTPUT e forçar LOW antes de qualquer outra
    // inicialização, garantindo que flutuações de boot não acionem
    // a válvula solenoide (CA-01).

    pinMode(PINO_RELE_CANAL_1, OUTPUT);
    digitalWrite(PINO_RELE_CANAL_1, LOW);
}

// Variáveis internas para controle de override manual, temporizadores e detecção de transições
static bool ultimoEstadoAgendado = false;
static bool manualOverrideActive = false;
static bool manualOverrideState  = false;
static unsigned long tempoInicioManualMs = 0; // Armazena timestamp de acionamento manual (RF1.4)

void avaliarAgendamentos(const ConfiguracaoIrrigacao& configIrrigacao, int minutoAtual) {
    // Avaliar sessões do Canal 1
    bool canal1DeveEstarLigado = false;
    for (uint8_t i = 0; i < MAX_SESSOES_POR_CANAL; i++) {
        const SessaoIrrigacao& sessao = configIrrigacao.canal1[i];
        if (!sessao.habilitada || sessao.duracao == 0) {
            continue; // Sessão desabilitada ou sem duração — ignorar
        }

        int minutosInicio = converterHorarioParaMinutos(sessao.inicio);
        if (minutosInicio < 0) {
            continue; // Formato inválido — ignorar silenciosamente
        }

        // Verificar se o minuto atual está dentro da janela [início, início+duração)
        if (minutoAtual >= minutosInicio && minutoAtual < (minutosInicio + sessao.duracao)) {
            // Se a duração da sessão for maior que 10 minutos, aplica o fracionamento (RF1.1)
            if (sessao.duracao > 10) {
                int minutoRelativo = minutoAtual - minutosInicio;
                // Ciclos de 5 min LIGADO e 10 min DESLIGADO (período de 15 minutos)
                canal1DeveEstarLigado = (minutoRelativo % 15) < 5;
            } else {
                canal1DeveEstarLigado = true;
            }
            break; // Basta uma sessão ativa para definir o estado do canal
        }
    }

    // Se houve transição no estado agendado, o override manual expira (retorna ao automático)
    if (canal1DeveEstarLigado != ultimoEstadoAgendado) {
        manualOverrideActive = false;
        ultimoEstadoAgendado = canal1DeveEstarLigado;
        Serial.println(F("[INFO] Transição de agendamento detectada. Retornando ao modo automático."));
    }

    // Limite de segurança manual de no máximo 10 minutos ligado (RF1.4)
    if (manualOverrideActive && manualOverrideState) {
        if (millis() - tempoInicioManualMs >= 600000UL) { // 10 minutos (600.000 ms)
            manualOverrideState = false;
            Serial.println(F("[WARN] Limite de segurança manual de 10 minutos atingido. Desligando a válvula solenoide."));
        }
    }

    // Grava no pino físico o estado de override ou o agendado
    bool estadoFinal = manualOverrideActive ? manualOverrideState : canal1DeveEstarLigado;
    digitalWrite(PINO_RELE_CANAL_1, estadoFinal ? HIGH : LOW);
}

void controleManual(uint8_t canal, bool estado) {
    uint8_t pino = obterPinoDoCanal(canal);
    if (pino == 0xFF) {
        Serial.print(F("[ERROR] Canal inválido para controle manual: "));
        Serial.println(canal);
        return;
    }

    // Ativar o override manual e registrar o estado desejado
    manualOverrideActive = true;
    manualOverrideState  = estado;

    if (estado) {
        tempoInicioManualMs = millis(); // Registra o início do temporizador de segurança
    }

    digitalWrite(pino, estado ? HIGH : LOW);

    Serial.print(F("[INFO] Override manual ativado — Canal "));
    Serial.print(canal);
    Serial.println(estado ? F(": LIGADO") : F(": DESLIGADO"));
}

bool obterEstadoRele(uint8_t canal) {
    uint8_t pino = obterPinoDoCanal(canal);
    if (pino == 0xFF) {
        return false;
    }
    return digitalRead(pino) == HIGH;
}

void obterProximaIrrigacao(const ConfiguracaoIrrigacao &config, int minutoAtual, char *buffer, size_t maxLen) {
    int menorDiff = 9999;
    const SessaoIrrigacao* proxima = nullptr;

    for (uint8_t i = 0; i < MAX_SESSOES_POR_CANAL; i++) {
        const SessaoIrrigacao& sessao = config.canal1[i];
        if (!sessao.habilitada || sessao.duracao == 0) continue;

        int minutosInicio = converterHorarioParaMinutos(sessao.inicio);
        if (minutosInicio < 0) continue;

        int diff = minutosInicio - minutoAtual;
        if (diff <= 0) {
            // Se já passou ou coincide com o minuto atual, o próximo ciclo será amanhã (+1 dia em minutos)
            diff += 1440;
        }

        if (diff < menorDiff) {
            menorDiff = diff;
            proxima = &sessao;
        }
    }

    if (proxima != nullptr) {
        strlcpy(buffer, proxima->inicio, maxLen);
    } else {
        strlcpy(buffer, "--:--", maxLen);
    }
}

void obterUltimaIrrigacao(const ConfiguracaoIrrigacao &config, int minutoAtual, char *buffer, size_t maxLen) {
    int menorDiff = 9999;
    const SessaoIrrigacao* ultima = nullptr;

    for (uint8_t i = 0; i < MAX_SESSOES_POR_CANAL; i++) {
        const SessaoIrrigacao& sessao = config.canal1[i];
        if (!sessao.habilitada || sessao.duracao == 0) continue;

        int minutosInicio = converterHorarioParaMinutos(sessao.inicio);
        if (minutosInicio < 0) continue;

        int diff = minutoAtual - minutosInicio;
        if (diff < 0) {
            // Se o horário de início é maior que o atual, a última execução ocorreu ontem (+1 dia em minutos)
            diff += 1440;
        }

        if (diff < menorDiff) {
            menorDiff = diff;
            ultima = &sessao;
        }
    }

    if (ultima != nullptr) {
        strlcpy(buffer, ultima->inicio, maxLen);
    } else {
        strlcpy(buffer, "--:--", maxLen);
    }
}
