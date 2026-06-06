#pragma once

/**
 * relogioNtp.h — Interface do Módulo de Sincronização NTP
 *
 * Gerencia o relógio do sistema sincronizado via NTP.
 * Fornece funções para obter hora, data e conversão para "minutos
 * desde meia-noite" (usado pelo Scheduler no ADR-005).
 *
 * Em caso de falha de rede, o relógio local interno mantém a
 * contagem de tempo baseada no último sync bem-sucedido (RF1.2).
 *
 * Requisitos atendidos: RF1.2, ADR-004 (campos NTP).
 */

#include "estruturas.h"

/**
 * Inicializa o cliente NTP com os parâmetros da configuração de rede.
 *
 * Configura o servidor NTP, fuso horário e intervalo de sincronização
 * conforme os valores carregados do /network.json (ADR-004).
 *
 * @param configRede Referência constante com as configurações de NTP.
 */
void inicializarNtp(const ConfiguracaoRede& configRede);

/**
 * Atualiza o relógio NTP.
 *
 * Deve ser chamada no loop() principal. O NTPClient gerencia
 * internamente o intervalo de resincronização. Em caso de falha
 * de rede, o relógio local continua operando (RF1.2).
 */
void atualizarNtp();

/**
 * Retorna a hora atual formatada como "HH:MM:SS".
 * Inline para eliminar overhead no loop de atualização da UI.
 */
String obterHoraFormatada();

/**
 * Retorna a data atual formatada para exibição no dashboard.
 * Ex: "05 de Junho de 2026".
 */
String obterDataFormatada();

/**
 * Converte o horário atual em minutos desde meia-noite (0–1439).
 *
 * Usado pelo motor de agendamento (ADR-005) para comparação eficiente
 * com os horários das sessões de irrigação, evitando parsing de strings.
 *
 * Exemplo: 06:30 → 390 minutos.
 *
 * @return Valor inteiro de 0 a 1439 representando o minuto atual do dia.
 */
inline int obterMinutosDesdeMeiaNoite();
