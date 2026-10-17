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
#include "relogioNtp.h"

// Variáveis locais para armazenar o horário da última irrigação real na RAM
static char horarioUltimaIrrigacaoRAM[6] = "--:--";

// Protótipos de funções auxiliares internas
static void setarEstadoReleFisico(bool ligado);


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

// Variáveis internas para controle de override manual, temporizadores e detecção de transições
static bool ultimoEstadoAgendado = false;
static bool manualOverrideActive = false;
static bool manualOverrideState  = false;
static unsigned long tempoInicioManualMs = 0; // Armazena timestamp de acionamento manual (RF1.4)

// Variáveis para desacoplamento assíncrono do acionamento do relé (ADR-015)
static bool acionamentoPendente = false;
static bool estadoPendente = false;
static unsigned long tempoSolicitacaoAcionamentoMs = 0;

// Variáveis para auto-verificação e proteção contra disparos espúrios por ruído elétrico (RF6.3, ADR-012)
static uint32_t disparosEspuriosCount = 0;
static bool estadoAutorizadoAtual = false;

void inicializarReles() {
    // ADR-007: Esta é a primeira ação do setup().
    // Configurar como OUTPUT e forçar LOW antes de qualquer outra
    // inicialização, garantindo que flutuações de boot no GPIO 0 não acionem
    // a válvula solenoide (CA-01).

    pinMode(PINO_RELE_CANAL_1, OUTPUT);
    digitalWrite(PINO_RELE_CANAL_1, ESTADO_RELE_DESLIGADO);
    estadoAutorizadoAtual = false;
}

uint32_t obterContadorDisparosEspurios() {
    return disparosEspuriosCount;
}

void verificarEProtegerEstadoReleFisico() {
    // Se o pino físico estiver no estado ligado mas sem autorização do firmware (RF6.3, CA-08)
    if (digitalRead(PINO_RELE_CANAL_1) == ESTADO_RELE_LIGADO && !estadoAutorizadoAtual) {
        digitalWrite(PINO_RELE_CANAL_1, ESTADO_RELE_DESLIGADO);
        disparosEspuriosCount++;
        Serial.print(F("[ERROR] Disparo espúrio por ruído elétrico detectado no GPIO 0! Válvula desligada imediatamente. Total de disparos espúrios: "));
        Serial.println(disparosEspuriosCount);
    }
}

static int obterDuracaoEsticada(int duracao) {
    if (duracao <= 10) {
        return duracao;
    }
    int q = duracao / 5;
    int r = duracao % 5;
    if (r == 0) {
        return (q - 1) * 15 + 5;
    } else {
        return q * 15 + r;
    }
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

        int duracaoEsticada = obterDuracaoEsticada(sessao.duracao);
        // Verificar se o minuto atual está dentro da janela [início, início+duraçãoEsticada)
        if (minutoAtual >= minutosInicio && minutoAtual < (minutosInicio + duracaoEsticada)) {
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

    // ADR-015: Se houver acionamento manual pendente aguardando a janela de desacoplamento,
    // não permite que o agendador periódico sobrescreva o estado do relé antes do tempo.
    if (acionamentoPendente) {
        return;
    }

    // Grava no pino físico o estado de override ou o agendado
    bool estadoFinal = manualOverrideActive ? manualOverrideState : canal1DeveEstarLigado;
    setarEstadoReleFisico(estadoFinal);
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

    // ADR-015: Desacoplamento assíncrono do acionamento físico do relé.
    // Em vez de efetuar digitalWrite síncrono imediatamente dentro do callback MQTT/HTTP,
    // registra a intenção e agenda o chaveamento físico para ocorrer no Super Loop
    // após uma janela de proteção de 150ms.
    acionamentoPendente = true;
    estadoPendente = estado;
    tempoSolicitacaoAcionamentoMs = millis();

    Serial.print(F("[INFO] Override manual registrado — Canal "));
    Serial.print(canal);
    Serial.print(estado ? F(": LIGADO") : F(": DESLIGADO"));
    Serial.print(F(" (chaveamento físico agendado em "));
    Serial.print(ATRASO_DESACOPLAMENTO_RELE_MS);
    Serial.println(F(" ms)"));
}

void processarAcionamentoRele() {
    // ADR-015: Executa o chaveamento físico do relé após a expiração da janela de desacoplamento
    if (acionamentoPendente && (millis() - tempoSolicitacaoAcionamentoMs >= ATRASO_DESACOPLAMENTO_RELE_MS)) {
        acionamentoPendente = false;
        setarEstadoReleFisico(estadoPendente);
        Serial.print(F("[INFO] Acionamento desacoplado do relé executado no pino físico GPIO 0: "));
        Serial.println(estadoPendente ? F("LIGADO") : F("DESLIGADO"));
    }
}

/**
 * Função interna centralizada para acionamento físico do relé.
 * Detecta a transição de desligado para ligado e registra a hora no formato HH:MM (NTP).
 */
static void setarEstadoReleFisico(bool ligado) {
    estadoAutorizadoAtual = ligado;
    bool estadoAnterior = (digitalRead(PINO_RELE_CANAL_1) == ESTADO_RELE_LIGADO);
    
    // Escrever no pino apenas se houver mudança de estado para evitar chaveamento redundante
    if (ligado != estadoAnterior) {
        digitalWrite(PINO_RELE_CANAL_1, ligado ? ESTADO_RELE_LIGADO : ESTADO_RELE_DESLIGADO);
        
        // Se a válvula acabou de ligar (transição de desligado para ligado)
        if (!estadoAnterior && ligado) {
            String horaCompleta = obterHoraFormatada(); // Retorna "HH:MM:SS"
            if (horaCompleta.length() >= 5) {
                strlcpy(horarioUltimaIrrigacaoRAM, horaCompleta.c_str(), 6);
                Serial.print(F("[INFO] Válvula ligada. Horário registrado na RAM: "));
                Serial.println(horarioUltimaIrrigacaoRAM);
            }
        }
    }
}

bool obterEstadoRele(uint8_t canal) {
    uint8_t pino = obterPinoDoCanal(canal);
    if (pino == 0xFF) {
        return false;
    }
    // ADR-015: Se houver comando manual pendente na janela de desacoplamento,
    // retorna a intenção solicitada para imediata responsividade da UI/telemetria.
    if (acionamentoPendente) {
        return estadoPendente;
    }
    return digitalRead(pino) == ESTADO_RELE_LIGADO;
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
    // Se o horário de última irrigação foi registrado na RAM, prioriza esse valor
    if (strcmp(horarioUltimaIrrigacaoRAM, "--:--") != 0) {
        strlcpy(buffer, horarioUltimaIrrigacaoRAM, maxLen);
        return;
    }

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
