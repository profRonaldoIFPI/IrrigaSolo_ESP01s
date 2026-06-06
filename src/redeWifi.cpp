/**
 * redeWifi.cpp — Implementação do Módulo de Gerenciamento Wi-Fi
 *
 * Gerencia os modos STA e AP do ESP8266, incluindo:
 *   - Desabilitação do cache nativo de credenciais (ADR-003)
 *   - Tentativa de conexão STA com timeout não-bloqueante
 *   - Fallback automático para AP em caso de falha (RF2.3)
 *   - Indicador visual via LED onboard (piscando = AP, fixo = STA)
 *
 * Nota sobre WiFi.persistent(false): impede que o SDK grave credenciais
 * na Flash interna. Assim, o LittleFS (/network.json) permanece como
 * a única fonte de verdade para credenciais de rede.
 */

#include "redeWifi.h"
#include <ESP8266WiFi.h>

// ---------------------------------------------------------------------------
// Estado interno do módulo
// ---------------------------------------------------------------------------

/** Flag que indica se o dispositivo está operando em modo AP */
static bool modoApAtivo = false;

/** Timestamp do último toggle do LED em modo AP (para piscar) */
static unsigned long ultimoToggleLedMs = 0;

/** Estado atual do LED para controle de pisca-pisca */
static bool estadoLedAp = false;

/** Cópia local das credenciais para reconexão e fallback */
static ConfiguracaoRede configRedeLocal;

// ---------------------------------------------------------------------------
// Funções auxiliares internas
// ---------------------------------------------------------------------------

/**
 * Inicia o modo Access Point com as credenciais configuradas.
 * Usado na primeira inicialização (sem rede) ou como fallback (RF2.3).
 * O IP padrão do AP é 192.168.4.1.
 */
static void iniciarModoAp() {
    WiFi.mode(WIFI_AP);
    WiFi.softAP(configRedeLocal.ssidAp, configRedeLocal.senhaAp);
    modoApAtivo = true;

    Serial.print(F("[INFO] Modo AP ativado. SSID: "));
    Serial.println(configRedeLocal.ssidAp);
    Serial.print(F("[INFO] IP do AP: "));
    Serial.println(WiFi.softAPIP().toString());

    // Configurar LED para piscar (lógica invertida no ESP8266)
    pinMode(PINO_LED_STATUS, OUTPUT);
    digitalWrite(PINO_LED_STATUS, HIGH); // HIGH = apagado (lógica invertida)
}

/**
 * Tenta conectar à rede local (modo STA) com timeout não-bloqueante.
 * Usa yield() a cada iteração para alimentar o WDT e manter as pilhas
 * de rede operacionais durante a espera.
 *
 * @return true se a conexão foi estabelecida dentro do timeout.
 */
static bool tentarConexaoSta() {
    Serial.print(F("[INFO] Tentando conectar à rede: "));
    Serial.println(configRedeLocal.ssidSta);

    WiFi.mode(WIFI_STA);
    WiFi.begin(configRedeLocal.ssidSta, configRedeLocal.senhaSta);

    unsigned long inicioTentativa = millis();

    while (WiFi.status() != WL_CONNECTED) {
        // Timeout atingido — conexão falhou
        if (millis() - inicioTentativa >= TIMEOUT_CONEXAO_STA_MS) {
            Serial.println(F("\n[WARN] Timeout de conexão STA atingido."));
            return false;
        }
        yield(); // Alimenta o WDT e processa pilhas de rede
        delay(100); // Pequeno intervalo para não saturar o loop
    }

    modoApAtivo = false;

    Serial.println();
    Serial.print(F("[INFO] Conectado à rede STA. IP: "));
    Serial.println(WiFi.localIP().toString());

    // LED fixo aceso (LOW = aceso na lógica invertida do ESP8266)
    pinMode(PINO_LED_STATUS, OUTPUT);
    digitalWrite(PINO_LED_STATUS, LOW);

    return true;
}

// ---------------------------------------------------------------------------
// Implementações públicas
// ---------------------------------------------------------------------------

void inicializarWifi(const ConfiguracaoRede& configRede) {
    // Guardar cópia local para uso no fallback e reconexão
    configRedeLocal = configRede;

    // ADR-003: Desabilitar o cache nativo de credenciais do SDK.
    // Garante que o LittleFS é a única fonte de verdade.
    WiFi.persistent(false);

    // Se há um SSID STA configurado, tentar conectar
    if (strlen(configRedeLocal.ssidSta) > 0) {
        if (!tentarConexaoSta()) {
            // Falha na conexão STA — fallback para AP (RF2.1, RF2.3)
            Serial.println(F("[WARN] Fallback para modo AP."));
            iniciarModoAp();
        }
    } else {
        // Nenhuma rede STA configurada — iniciar direto em AP (RF2.1)
        Serial.println(F("[INFO] Nenhuma rede STA configurada. Iniciando em modo AP."));
        iniciarModoAp();
    }
}

void verificarConexaoWifi() {
    // Se está em modo AP, gerenciar o LED piscante
    if (modoApAtivo) {
        unsigned long agora = millis();
        if (agora - ultimoToggleLedMs >= INTERVALO_LED_AP_MS) {
            ultimoToggleLedMs = agora;
            estadoLedAp = !estadoLedAp;
            // Lógica invertida: LOW = aceso, HIGH = apagado
            digitalWrite(PINO_LED_STATUS, estadoLedAp ? LOW : HIGH);
        }
        return; // Em modo AP, não tenta reconectar
    }

    // Em modo STA, verificar se a conexão caiu
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println(F("[WARN] Conexão STA perdida. Ativando fallback para modo AP (RF2.3)."));

        // Desconectar limpo antes de mudar de modo
        WiFi.disconnect();

        // Ativar AP sem interromper agendamentos carregados em memória (RF2.3)
        iniciarModoAp();
    }
}

inline bool estaConectadoSta() {
    return !modoApAtivo && (WiFi.status() == WL_CONNECTED);
}

String obterIpAtual() {
    if (modoApAtivo) {
        return WiFi.softAPIP().toString();
    }
    return WiFi.localIP().toString();
}
