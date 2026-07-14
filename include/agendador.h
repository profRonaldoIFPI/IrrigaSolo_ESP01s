#pragma once

/**
 * agendador.h — Interface do Motor de Agendamento (Scheduler)
 *
 * Módulo central do firmware. Responsável por:
 *   - Inicializar os relés em estado seguro (ADR-007, CA-01)
 *   - Avaliar as sessões de irrigação via Super Loop polling (ADR-005)
 *   - Validar conflitos de horários entre canais (ADR-006, RF3)
 *   - Fornecer controle manual sem interferir na automação (RF4.1)
 *
 * A lógica do agendador não acumula regas perdidas (RF1.3):
 * se o minuto de início já passou, o sistema simplesmente aguarda
 * o próximo horário programado.
 *
 * Requisitos atendidos: RF1.1, RF1.3, RF3.1, RF3.2, RF4.1,
 *                       CA-01, CA-02, CA-03, ADR-005, ADR-006, ADR-007.
 */

#include "estruturas.h"

/**
 * Configura os pinos GPIO dos relés como OUTPUT e os força para LOW.
 *
 * Esta DEVE ser a primeira função chamada no setup(), antes mesmo
 * de inicializar Serial, LittleFS ou Wi-Fi (ADR-007). Garante que
 * picos de flutuação durante o boot não acionem válvulas (CA-01).
 */
void inicializarReles();

/**
 * Avalia todas as sessões de irrigação ativas e aciona/desliga o relé.
 *
 * Para cada sessão habilitada, calcula se o minuto atual está dentro
 * da janela [início, início + duração). Se sim, liga o relé; se não,
 * desliga. Sessões desabilitadas são ignoradas (CA-02).
 *
 * Não acumula regas perdidas: se o boot ocorreu após o horário de
 * início, o sistema não compensa — apenas aguarda o próximo (RF1.3).
 *
 * @param configIrrigacao Configuração completa de irrigação.
 * @param minutoAtual     Minutos desde meia-noite (0–1439), obtido do NTP.
 */
void avaliarAgendamentos(const ConfiguracaoIrrigacao &configIrrigacao,
                         int minutoAtual);

/**
 * Aciona ou desliga manualmente um canal de irrigação.
 *
 * Este controle é independente da programação automática (RF4.1).
 * O Scheduler continuará avaliando horários normalmente no próximo
 * ciclo do loop(), podendo sobrescrever o estado manual se houver
 * uma sessão ativa para aquele minuto.
 *
 * @param canal  Número do canal (1 ou 2).
 * @param estado true para ligar, false para desligar.
 */
void controleManual(uint8_t canal, bool estado);

/**
 * Retorna o estado atual (HIGH/LOW) de um relé.
 * Inline para eliminar overhead no polling do loop().
 *
 * @param canal Número do canal (1 ou 2).
 * @return true se o relé está ligado (HIGH), false se desligado (LOW).
 */
bool obterEstadoRele(uint8_t canal);

/**
 * Obtém o horário de início da próxima irrigação programada.
 *
 * @param config Configuração completa de irrigação.
 * @param minutoAtual Minutos desde meia-noite (NTP).
 * @param buffer Buffer de saída para o horário "HH:MM".
 * @param maxLen Tamanho máximo do buffer de saída.
 */
void obterProximaIrrigacao(const ConfiguracaoIrrigacao &config, int minutoAtual, char *buffer, size_t maxLen);

/**
 * Obtém o horário de início da última irrigação programada.
 *
 * @param config Configuração completa de irrigação.
 * @param minutoAtual Minutos desde meia-noite (NTP).
 * @param buffer Buffer de saída para o horário "HH:MM".
 * @param maxLen Tamanho máximo do buffer de saída.
 */
void obterUltimaIrrigacao(const ConfiguracaoIrrigacao &config, int minutoAtual, char *buffer, size_t maxLen);
