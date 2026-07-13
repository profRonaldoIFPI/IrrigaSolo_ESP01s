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
            canal1DeveEstarLigado = true;
            break; // Basta uma sessão ativa para ligar o canal
        }
    }
    digitalWrite(PINO_RELE_CANAL_1, canal1DeveEstarLigado ? HIGH : LOW);
}

void controleManual(uint8_t canal, bool estado) {
    uint8_t pino = obterPinoDoCanal(canal);
    if (pino == 0xFF) {
        Serial.print(F("[ERROR] Canal inválido para controle manual: "));
        Serial.println(canal);
        return;
    }

    digitalWrite(pino, estado ? HIGH : LOW);

    Serial.print(F("[INFO] Controle manual — Canal "));
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
