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
 * Validação de conflitos (ADR-006):
 *   Quando permitirSimultaneidade == false, duas sessões (de canais
 *   diferentes) conflitam se suas janelas de minutos se sobrepõem.
 *   A verificação é feita no backend antes de salvar, impedindo que
 *   um POST malicioso ou falha no frontend crie horários perigosos.
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
        case 2: return PINO_RELE_CANAL_2;
        default: return 0xFF;
    }
}

/**
 * Verifica se duas janelas de minutos se sobrepõem.
 * Usado na validação de conflitos (ADR-006).
 *
 * @param inicio1  Minuto de início da janela 1.
 * @param duracao1 Duração em minutos da janela 1.
 * @param inicio2  Minuto de início da janela 2.
 * @param duracao2 Duração em minutos da janela 2.
 * @return true se há sobreposição.
 */
static bool janelasSeOverpoem(int inicio1, uint8_t duracao1, int inicio2, uint8_t duracao2) {
    int fim1 = inicio1 + duracao1;
    int fim2 = inicio2 + duracao2;

    // Duas janelas [a, b) e [c, d) se sobrepõem se a < d && c < b
    return (inicio1 < fim2) && (inicio2 < fim1);
}

// ---------------------------------------------------------------------------
// Implementações públicas
// ---------------------------------------------------------------------------

void inicializarReles() {
    // ADR-007: Esta é a primeira ação do setup().
    // Configurar como OUTPUT e forçar LOW antes de qualquer outra
    // inicialização, garantindo que flutuações de boot não acionem
    // as válvulas solenoides (CA-01).

    pinMode(PINO_RELE_CANAL_1, OUTPUT);
    digitalWrite(PINO_RELE_CANAL_1, LOW);

    pinMode(PINO_RELE_CANAL_2, OUTPUT);
    digitalWrite(PINO_RELE_CANAL_2, LOW);
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

    // Avaliar sessões do Canal 2
    bool canal2DeveEstarLigado = false;
    for (uint8_t i = 0; i < MAX_SESSOES_POR_CANAL; i++) {
        const SessaoIrrigacao& sessao = configIrrigacao.canal2[i];
        if (!sessao.habilitada || sessao.duracao == 0) {
            continue;
        }

        int minutosInicio = converterHorarioParaMinutos(sessao.inicio);
        if (minutosInicio < 0) {
            continue;
        }

        if (minutoAtual >= minutosInicio && minutoAtual < (minutosInicio + sessao.duracao)) {
            canal2DeveEstarLigado = true;
            break;
        }
    }
    digitalWrite(PINO_RELE_CANAL_2, canal2DeveEstarLigado ? HIGH : LOW);
}

bool validarConflitos(const ConfiguracaoIrrigacao& configIrrigacao) {
    // Se simultaneidade é permitida, não há conflitos a verificar
    if (configIrrigacao.permitirSimultaneidade) {
        return true;
    }

    // Comparar cada sessão habilitada do Canal 1 com cada sessão do Canal 2
    for (uint8_t i = 0; i < MAX_SESSOES_POR_CANAL; i++) {
        const SessaoIrrigacao& s1 = configIrrigacao.canal1[i];
        if (!s1.habilitada || s1.duracao == 0) {
            continue;
        }

        int inicio1 = converterHorarioParaMinutos(s1.inicio);
        if (inicio1 < 0) {
            continue;
        }

        for (uint8_t j = 0; j < MAX_SESSOES_POR_CANAL; j++) {
            const SessaoIrrigacao& s2 = configIrrigacao.canal2[j];
            if (!s2.habilitada || s2.duracao == 0) {
                continue;
            }

            int inicio2 = converterHorarioParaMinutos(s2.inicio);
            if (inicio2 < 0) {
                continue;
            }

            if (janelasSeOverpoem(inicio1, s1.duracao, inicio2, s2.duracao)) {
                // ADR-006: Registrar qual par de sessões conflitou
                Serial.print(F("[WARN] Conflito detectado: Canal1["));
                Serial.print(i);
                Serial.print(F("] ("));
                Serial.print(s1.inicio);
                Serial.print(F("+"));
                Serial.print(s1.duracao);
                Serial.print(F("min) x Canal2["));
                Serial.print(j);
                Serial.print(F("] ("));
                Serial.print(s2.inicio);
                Serial.print(F("+"));
                Serial.print(s2.duracao);
                Serial.println(F("min)"));
                return false; // Conflito encontrado
            }
        }
    }

    return true; // Nenhum conflito
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

inline bool obterEstadoRele(uint8_t canal) {
    uint8_t pino = obterPinoDoCanal(canal);
    if (pino == 0xFF) {
        return false;
    }
    return digitalRead(pino) == HIGH;
}
