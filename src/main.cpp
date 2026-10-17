/**
 * main.cpp — Ponto de Entrada do Firmware IrrigaSolo (ESP-01S)
 *
 * Orquestra a inicialização e o Super Loop principal do sistema adaptado
 * para o microcontrolador ESP8266 ESP-01S integrado ao módulo relé v4.0.
 *
 * Sequência de inicialização (ADR-007, ADR-014):
 *   1. inicializarReles()       → GPIO 0 em LOW imediato e GPIO 2 apagado (CA-01)
 *   2. Serial.begin()           → Telemetria Serial a 115200 bps (ADR-008)
 *   3. inicializarLittleFS()    → Sistema de arquivos LittleFS (1MB Flash)
 *   4. carregarConfiguracoes()  → /network.json + /irrigation.json (ADR-004)
 *   5. inicializarWifi()        → Conexão STA com fallback AP (RF2, ADR-013)
 *   6. inicializarNtp()         → Sincronização de relógio via NTP (RF1.2)
 *   7. inicializarMqtt()        → Cliente MQTT não-bloqueante (RF5, ADR-010)
 *   8. inicializarServidor()    → Servidor HTTP na porta 80 e rota OTA /update
 *   9. inicializarAutovigilancia() → Watchdog e proteção contra travamentos
 *
 * Super Loop (loop):
 *   - verificarEProtegerEstadoReleFisico() → Auto-verificação anti-ruído elétrico
 *   - processarAcionamentoRele() → Desacoplamento assíncrono do relé (ADR-015)
 *   - processarRequisicoes()    → Atende clientes HTTP
 *   - verificarConexaoWifi()    → Monitora STA e reconexão periódica
 *   - atualizarNtp()            → Resincroniza relógio
 *   - processarMqtt()           → Processa fila e reconexão MQTT
 *   - processarAutovigilancia() → Watchdog de heap e reset de 24h
 *   - avaliarAgendamentos()     → Polling a cada 1s do motor de irrigação
 *   - publicarTelemetria()      → Publicação periódica de estado via MQTT
 */

#include "pinosHardware.h"
#include "estruturas.h"
#include "persistencia.h"
#include "redeWifi.h"
#include "relogioNtp.h"
#include "agendador.h"
#include "servidorWeb.h"
#include "clienteMqtt.h"
#include "autovigilancia.h"

// ---------------------------------------------------------------------------
// Variáveis globais do sistema
// ---------------------------------------------------------------------------

/** Configuração de rede carregada do LittleFS (ADR-004) */
static ConfiguracaoRede configRede;

/** Configuração de irrigação carregada do LittleFS (ADR-004) */
static ConfiguracaoIrrigacao configIrrigacao;

/** Timestamps das avaliações de tempo (controle via millis) */
static unsigned long ultimaAvaliacaoMs = 0;
static unsigned long ultimaPublicacaoMqttMs = 0;

/**
 * Flag que indica se a configuração de irrigação está válida.
 * Se false, o agendador não aciona relés automaticamente (RF6.2).
 */
static bool irrigacaoConfiguradaComSucesso = false;

// ---------------------------------------------------------------------------
// setup() — Inicialização do Sistema (ADR-007)
// ---------------------------------------------------------------------------

void setup() {
    // -----------------------------------------------------------------------
    // PASSO 1: Segurança elétrica — Relé em LOW ANTES de tudo (ADR-007, CA-01)
    // Garante que flutuações de boot no GPIO 0 não acionem a válvula.
    // -----------------------------------------------------------------------
    inicializarReles();

    // -----------------------------------------------------------------------
    // PASSO 2: Telemetria Serial (ADR-008)
    // -----------------------------------------------------------------------
    Serial.begin(VELOCIDADE_SERIAL);
    Serial.println();
    Serial.println(F("========================================"));
    Serial.println(F("  IrrigaSolo ESP-01S — Inicializando..."));
    Serial.println(F("========================================"));

    // -----------------------------------------------------------------------
    // PASSO 3: Sistema de Arquivos (LittleFS)
    // -----------------------------------------------------------------------
    if (!inicializarLittleFS()) {
        Serial.println(F("[ERROR] Boot sem LittleFS. Operando com defaults."));
    }

    // -----------------------------------------------------------------------
    // PASSO 4: Carregar Configurações (ADR-004)
    // -----------------------------------------------------------------------
    carregarConfiguracaoRede(configRede);
    irrigacaoConfiguradaComSucesso = carregarConfiguracaoIrrigacao(configIrrigacao);

    if (!irrigacaoConfiguradaComSucesso) {
        // RF6.2: Falha na configuração de irrigação — relés permanecem inativos
        Serial.println(F("[WARN] Irrigação operando com defaults seguros (relé inativo)."));
    }

    // -----------------------------------------------------------------------
    // PASSO 5: Wi-Fi (STA ou AP com fallback) — ADR-003, RF2
    // -----------------------------------------------------------------------
    inicializarWifi(configRede);

    // -----------------------------------------------------------------------
    // PASSO 6: NTP — Sincronização de Relógio (RF1.2)
    // -----------------------------------------------------------------------
    inicializarNtp(configRede);

    // -----------------------------------------------------------------------
    // PASSO 7: MQTT — Inicializar cliente (ADR-010, ADR-012, RF5)
    // -----------------------------------------------------------------------
    inicializarMqtt(configRede, &configIrrigacao);

    // -----------------------------------------------------------------------
    // PASSO 8: Servidor Web HTTP (última inicialização)
    // -----------------------------------------------------------------------
    inicializarServidor(&configRede, &configIrrigacao);

    // -----------------------------------------------------------------------
    // PASSO 9: Autovigilância (Watchdog de resiliência contra travamentos)
    // -----------------------------------------------------------------------
    inicializarAutovigilancia();

    Serial.println(F("========================================"));
    Serial.println(F("  IrrigaSolo pronto para operar."));
    Serial.println(F("========================================"));
}

// ---------------------------------------------------------------------------
// loop() — Super Loop Não-Bloqueante (ADR-005)
// ---------------------------------------------------------------------------

void loop() {
    // Auto-verificação imediata do pino contra disparos espúrios por ruído elétrico (RF6.3, CA-08)
    verificarEProtegerEstadoReleFisico();

    // ADR-015: Processar chaveamento físico do relé com desacoplamento assíncrono (protege Wi-Fi contra EMI)
    processarAcionamentoRele();

    // Atender requisições HTTP pendentes
    processarRequisicoes();

    // Monitorar estado da conexão Wi-Fi e fallback AP (RF2.3)
    verificarConexaoWifi();

    // Atualizar relógio NTP (o cliente gerencia o intervalo internamente)
    atualizarNtp();

    // Processar loop de conexão e escuta do MQTT (RF5.2)
    processarMqtt();

    // Executar rotinas do módulo de autovigilância contra travamentos
    processarAutovigilancia();

    unsigned long agora = millis();

    // Avaliar agendamentos a cada INTERVALO_AVALIACAO_MS (1 segundo)
    if (agora - ultimaAvaliacaoMs >= INTERVALO_AVALIACAO_MS) {
        ultimaAvaliacaoMs = agora;

        if (irrigacaoConfiguradaComSucesso) {
            int minutoAtual = obterMinutosDesdeMeiaNoite();
            avaliarAgendamentos(configIrrigacao, minutoAtual);
        }
    }

    // Publicar telemetria MQTT com intervalo configurável (RF5.1)
    if (agora - ultimaPublicacaoMqttMs >= (configRede.mqttInterval * 1000UL)) {
        ultimaPublicacaoMqttMs = agora;

        char ultimaIrrigacao[6] = "--:--";
        char proximaIrrigacao[6] = "--:--";
        int minutoAtual = obterMinutosDesdeMeiaNoite();
        obterUltimaIrrigacao(configIrrigacao, minutoAtual, ultimaIrrigacao, sizeof(ultimaIrrigacao));
        obterProximaIrrigacao(configIrrigacao, minutoAtual, proximaIrrigacao, sizeof(proximaIrrigacao));

        publicarTelemetria(obterEstadoRele(1), ultimaIrrigacao, proximaIrrigacao);
    }
}
