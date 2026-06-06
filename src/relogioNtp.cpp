/**
 * relogioNtp.cpp — Implementação do Módulo de Sincronização NTP
 *
 * Utiliza a biblioteca NTPClient para sincronizar o relógio do ESP8266
 * com um servidor de tempo remoto. O fuso horário e o intervalo de
 * sincronização são configuráveis via /network.json (ADR-004).
 *
 * Comportamento em falha de rede (RF1.2):
 *   O NTPClient mantém internamente um epoch baseado em millis(),
 *   permitindo que o relógio local continue operando com a última
 *   sincronização bem-sucedida como referência. A precisão degrada
 *   gradualmente (drift do oscilador do ESP8266: ~10 ppm), mas é
 *   suficiente para agendamentos no intervalo de minutos.
 */

#include "relogioNtp.h"
#include <WiFiUdp.h>
#include <NTPClient.h>

// ---------------------------------------------------------------------------
// Instâncias do módulo NTP
// ---------------------------------------------------------------------------

/** Socket UDP utilizado pelo NTPClient */
static WiFiUDP udpNtp;

/**
 * Cliente NTP. Inicializado com valores padrão e reconfigurado
 * em inicializarNtp() com os parâmetros do /network.json.
 */
static NTPClient clienteNtp(udpNtp);

// ---------------------------------------------------------------------------
// Nomes dos meses em Português para formatação de data
// ---------------------------------------------------------------------------
static const char* const NOMES_MESES[] = {
    "Janeiro", "Fevereiro", "Março",    "Abril",
    "Maio",    "Junho",     "Julho",    "Agosto",
    "Setembro","Outubro",   "Novembro", "Dezembro"
};

// ---------------------------------------------------------------------------
// Implementações públicas
// ---------------------------------------------------------------------------

void inicializarNtp(const ConfiguracaoRede& configRede) {
    // Configurar servidor NTP e offset de fuso horário em segundos
    // Ex: fusoHorario = -3 → offsetSegundos = -10800
    long offsetSegundos = (long)configRede.fusoHorario * 3600L;

    clienteNtp.setPoolServerName(configRede.servidorNtp);
    clienteNtp.setTimeOffset(offsetSegundos);
    clienteNtp.setUpdateInterval(configRede.intervaloSyncNtp * 1000UL);

    clienteNtp.begin();

    // Forçar uma primeira sincronização imediata
    if (clienteNtp.forceUpdate()) {
        Serial.print(F("[INFO] NTP sincronizado. Hora atual: "));
        Serial.println(clienteNtp.getFormattedTime());
    } else {
        Serial.println(F("[WARN] Falha na sincronização NTP inicial. Relógio local ativo (RF1.2)."));
    }
}

void atualizarNtp() {
    // O NTPClient gerencia internamente o intervalo de resincronização.
    // update() retorna silenciosamente se o intervalo não expirou.
    clienteNtp.update();
}

String obterHoraFormatada() {
    return clienteNtp.getFormattedTime();
}

String obterDataFormatada() {
    // Obter o epoch time e calcular a data manualmente
    // (NTPClient não fornece data diretamente)
    unsigned long epochTime = clienteNtp.getEpochTime();

    // Converter epoch para componentes de data usando time.h
    time_t rawTime = (time_t)epochTime;
    struct tm* timeInfo = gmtime(&rawTime);

    // O fuso já está aplicado pelo NTPClient no epoch, mas gmtime
    // não sabe disso. Como o offset já foi somado ao epoch pelo
    // NTPClient, o resultado de gmtime já reflete o horário local.

    char bufferData[32];
    snprintf(bufferData, sizeof(bufferData), "%02d de %s de %d",
             timeInfo->tm_mday,
             NOMES_MESES[timeInfo->tm_mon], // tm_mon: 0–11
             timeInfo->tm_year + 1900);

    return String(bufferData);
}

int obterMinutosDesdeMeiaNoite() {
    return clienteNtp.getHours() * 60 + clienteNtp.getMinutes();
}
