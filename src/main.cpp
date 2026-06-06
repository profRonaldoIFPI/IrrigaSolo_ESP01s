/**
 * main.cpp — Ponto de Entrada do Firmware IrrigaSolo
 *
 * Orquestra a inicialização e o Super Loop principal do sistema.
 *
 * Sequência de inicialização (ADR-007):
 *   1. inicializarReles()       → GPIO LOW imediato (CA-01)
 *   2. Serial.begin()           → Telemetria (ADR-008)
 *   3. inicializarLittleFS()    → Sistema de arquivos
 *   4. carregarConfiguracoes()  → /network.json + /irrigation.json
 *   5. inicializarWifi()        → STA ou fallback AP (RF2)
 *   6. inicializarNtp()         → Sincronização de relógio (RF1.2)
 *   7. inicializarServidor()    → Servidor HTTP na porta 80
 *
 * Super Loop (loop):
 *   - processarRequisicoes()    → Atende clientes HTTP
 *   - verificarConexaoWifi()    → Monitora STA, fallback AP
 *   - atualizarNtp()            → Resincroniza relógio
 *   - avaliarAgendamentos()     → Polling a cada 1s (ADR-005)
 *
 * Requisitos atendidos: Todos (integração final).
 */

#include "pinosHardware.h"
#include "estruturas.h"
#include "persistencia.h"
#include "redeWifi.h"
#include "relogioNtp.h"
#include "agendador.h"
#include "servidorWeb.h"

// ---------------------------------------------------------------------------
// Variáveis globais do sistema
// ---------------------------------------------------------------------------

/** Configuração de rede carregada do LittleFS (ADR-004) */
static ConfiguracaoRede configRede;

/** Configuração de irrigação carregada do LittleFS (ADR-004) */
static ConfiguracaoIrrigacao configIrrigacao;

/** Timestamp da última avaliação de agendamentos (para controle via millis) */
static unsigned long ultimaAvaliacaoMs = 0;

/**
 * Flag que indica se a configuração de irrigação está válida.
 * Se false, o agendador não aciona relés automaticamente (RF5.2).
 */
static bool irrigacaoConfiguradaComSucesso = false;

// ---------------------------------------------------------------------------
// setup() — Inicialização do Sistema (ADR-007)
// ---------------------------------------------------------------------------

void setup() {
    // -----------------------------------------------------------------------
    // PASSO 1: Segurança elétrica — Relés em LOW ANTES de tudo (ADR-007, CA-01)
    // Garante que flutuações de GPIO durante o boot não acionem válvulas.
    // -----------------------------------------------------------------------
    inicializarReles();

    // -----------------------------------------------------------------------
    // PASSO 2: Telemetria Serial (ADR-008)
    // -----------------------------------------------------------------------
    Serial.begin(VELOCIDADE_SERIAL);
    Serial.println();
    Serial.println(F("========================================"));
    Serial.println(F("  IrrigaSolo v1.0 — Inicializando..."));
    Serial.println(F("========================================"));

    // -----------------------------------------------------------------------
    // PASSO 3: Sistema de Arquivos (LittleFS)
    // -----------------------------------------------------------------------
    if (!inicializarLittleFS()) {
        Serial.println(F("[ERROR] Boot sem LittleFS. Operando com defaults."));
        // Continua com defaults — o sistema será funcional em modo AP
    }

    // -----------------------------------------------------------------------
    // PASSO 4: Carregar Configurações (ADR-004)
    // -----------------------------------------------------------------------
    carregarConfiguracaoRede(configRede);
    irrigacaoConfiguradaComSucesso = carregarConfiguracaoIrrigacao(configIrrigacao);

    if (!irrigacaoConfiguradaComSucesso) {
        // RF5.2: Falha na configuração de irrigação — relés permanecem inativos
        Serial.println(F("[WARN] Irrigação operando com defaults seguros (relés inativos)."));
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
    // PASSO 7: Servidor Web HTTP (última inicialização)
    // Passa ponteiros para as structs de configuração para que os endpoints
    // possam ler e modificar os dados em tempo real.
    // -----------------------------------------------------------------------
    inicializarServidor(&configRede, &configIrrigacao);

    Serial.println(F("========================================"));
    Serial.println(F("  IrrigaSolo pronto para operar."));
    Serial.println(F("========================================"));
}

// ---------------------------------------------------------------------------
// loop() — Super Loop Não-Bloqueante (ADR-005)
// ---------------------------------------------------------------------------

void loop() {
    // Atender requisições HTTP pendentes (não-bloqueante)
    processarRequisicoes();

    // Monitorar estado da conexão Wi-Fi e fallback AP (RF2.3)
    verificarConexaoWifi();

    // Atualizar relógio NTP (o cliente gerencia o intervalo internamente)
    atualizarNtp();

    // Avaliar agendamentos a cada INTERVALO_AVALIACAO_MS (1 segundo)
    // Apenas se a configuração de irrigação foi carregada com sucesso.
    unsigned long agora = millis();
    if (agora - ultimaAvaliacaoMs >= INTERVALO_AVALIACAO_MS) {
        ultimaAvaliacaoMs = agora;

        if (irrigacaoConfiguradaComSucesso) {
            int minutoAtual = obterMinutosDesdeMeiaNoite();
            avaliarAgendamentos(configIrrigacao, minutoAtual);
        }
    }
}
