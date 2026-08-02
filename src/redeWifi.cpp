/**
 * redeWifi.cpp — Implementação do Módulo de Gerenciamento Wi-Fi
 *
 * Gerencia os modos STA e AP do ESP8266, incluindo:
 *   - Desabilitação do cache nativo de credenciais (ADR-003)
 *   - Tentativa de conexão STA com timeout não-bloqueante
 *   - Fallback automático para AP em caso de falha (RF2.3)
 *   - Auto-reconexão periódica não-bloqueante a cada 20s em caso de queda do roteador (ADR-013)
 *   - Indicador visual via LED onboard (piscando = AP, fixo = STA)
 */

#include "redeWifi.h"
#include <ESP8266WiFi.h>

// ---------------------------------------------------------------------------
// Estado interno do módulo
// ---------------------------------------------------------------------------

/** Flag que indica se o dispositivo está operando em modo AP / Dual */
static bool modoApAtivo = false;

/** Timestamp do último toggle do LED em modo AP (para piscar) */
static unsigned long ultimoToggleLedMs = 0;

/** Timestamp da última tentativa de reconexão STA em segundo plano (ADR-013) */
static unsigned long ultimaTentativaReconexaoStaMs = 0;
static const unsigned long INTERVALO_RECONEXAO_STA_MS = 20000; // 20 segundos

/** Estado atual do LED para controle de pisca-pisca */
static bool estadoLedAp = false;

/** Cópia local das credenciais para reconexão e fallback */
static ConfiguracaoRede configRedeLocal;

// ---------------------------------------------------------------------------
// Funções auxiliares internas
// ---------------------------------------------------------------------------

/**
 * Inicia o modo Access Point com as credenciais configuradas.
 * Usado na primeira inicialização (sem rede) ou como fallback (RF2.3, ADR-013).
 * O IP padrão do AP é 192.168.4.1.
 */
static void iniciarModoAp() {
    // Se há credenciais STA configuradas, habilita modo Dual (AP + STA) para permitir reconexão em background
    if (strlen(configRedeLocal.ssidSta) > 0) {
        WiFi.mode(WIFI_AP_STA);
    } else {
        WiFi.mode(WIFI_AP);
    }

    WiFi.softAP(configRedeLocal.ssidAp, configRedeLocal.senhaAp);
    modoApAtivo = true;

    Serial.print(F("[INFO] Modo AP/Dual ativado. SSID AP: "));
    Serial.println(configRedeLocal.ssidAp);
    Serial.print(F("[INFO] IP do AP: "));
    Serial.println(WiFi.softAPIP().toString());

    // Configurar LED para piscar (lógica invertida no ESP8266)
    pinMode(PINO_LED_STATUS, OUTPUT);
    digitalWrite(PINO_LED_STATUS, HIGH); // HIGH = apagado (lógica invertida)
}

/**
 * Tenta conectar à rede local (modo STA) com timeout não-bloqueante.
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
    WiFi.persistent(false);

    // Se há um SSID STA configurado, tentar conectar
    if (strlen(configRedeLocal.ssidSta) > 0) {
        if (!tentarConexaoSta()) {
            // Falha na conexão STA — fallback para AP (RF2.1, RF2.3)
            Serial.println(F("[WARN] Fallback para modo AP/Dual."));
            iniciarModoAp();
        }
    } else {
        // Nenhuma rede STA configurada — iniciar direto em AP (RF2.1)
        Serial.println(F("[INFO] Nenhuma rede STA configurada. Iniciando em modo AP."));
        iniciarModoAp();
    }
}

void verificarConexaoWifi() {
    unsigned long agora = millis();

    // 1. Se estiver operando em Modo AP / Dual (queda de sinal ou inicialização sem rede)
    if (modoApAtivo) {
        // Gerenciar o LED piscante em modo AP
        if (agora - ultimoToggleLedMs >= INTERVALO_LED_AP_MS) {
            ultimoToggleLedMs = agora;
            estadoLedAp = !estadoLedAp;
            // Lógica invertida: LOW = aceso, HIGH = apagado
            digitalWrite(PINO_LED_STATUS, estadoLedAp ? LOW : HIGH);
        }

        // Se houver SSID STA configurado, tentar reconectar periodicamente sem bloquear o loop (ADR-013)
        if (strlen(configRedeLocal.ssidSta) > 0) {
            // Verificar se a reconexão foi estabelecida no background
            if (WiFi.status() == WL_CONNECTED) {
                modoApAtivo = false;
                WiFi.mode(WIFI_STA); // Desativa o AP de fallback e passa para STA estrito
                pinMode(PINO_LED_STATUS, OUTPUT);
                digitalWrite(PINO_LED_STATUS, LOW); // LED fixo aceso

                Serial.println();
                Serial.print(F("[INFO] Auto-recuperação STA bem-sucedida! Reconectado à rede. IP: "));
                Serial.println(WiFi.localIP().toString());
                return;
            }

            // Tentar nova reconexão a cada 20 segundos
            if (agora - ultimaTentativaReconexaoStaMs >= INTERVALO_RECONEXAO_STA_MS) {
                ultimaTentativaReconexaoStaMs = agora;
                Serial.print(F("[INFO] Tentando reconectar à rede STA em segundo plano: "));
                Serial.println(configRedeLocal.ssidSta);
                WiFi.begin(configRedeLocal.ssidSta, configRedeLocal.senhaSta);
            }
        }
        return;
    }

    // 2. Em modo STA normal, se a conexão for perdida (ex: roteador reiniciou)
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println(F("[WARN] Conexão STA perdida (roteador reiniciando). Ativando modo dual AP/STA de auto-recuperação (RF2.3, ADR-013)."));

        // Ativar AP de emergência sem interromper agendamentos em memória
        iniciarModoAp();
        ultimaTentativaReconexaoStaMs = agora;
    }
}

bool estaConectadoSta() {
    return !modoApAtivo && (WiFi.status() == WL_CONNECTED);
}

String obterIpAtual() {
    if (modoApAtivo) {
        return WiFi.softAPIP().toString();
    }
    return WiFi.localIP().toString();
}
