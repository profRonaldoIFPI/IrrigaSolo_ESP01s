#include "clienteMqtt.h"
#include "agendador.h"
#include "sensorFluxo.h"
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// Instâncias globais locais do módulo
static WiFiClient espClient;
static PubSubClient mqttClient(espClient);

static ConfiguracaoRede configRedeLocal;
static unsigned long ultimaTentativaReconexaoMs = 0;
static const unsigned long INTERVALO_RECONEXAO_MQTT_MS = 10000;

// A configuração MQTT agora lê os tópicos e Client ID dinamicamente do LittleFS

/**
 * Função de callback interna. Chamada pela PubSubClient quando chega
 * uma mensagem nos tópicos inscritos (ex: comando).
 */
static void callbackMqtt(char* topic, byte* payload, unsigned int length) {
    // Converter o payload binário para string com segurança
    char payloadStr[16];
    unsigned int tamanhoCopiar = length < (sizeof(payloadStr) - 1) ? length : (sizeof(payloadStr) - 1);
    memcpy(payloadStr, payload, tamanhoCopiar);
    payloadStr[tamanhoCopiar] = '\0';

    Serial.print(F("[MQTT] Mensagem recebida no tópico ["));
    Serial.print(topic);
    Serial.print(F("]: "));
    Serial.println(payloadStr);

    // Processamento de comandos (RF5.2)
    if (strcmp(payloadStr, "LIGAR") == 0) {
        controleManual(1, true);
    } else if (strcmp(payloadStr, "DESLIGAR") == 0) {
        controleManual(1, false);
    } else if (strcmp(payloadStr, "ZERAR") == 0) {
        zerarVolumeTotal();
    }
}

void inicializarMqtt(const ConfiguracaoRede& config) {
    configRedeLocal = config;

    mqttClient.setServer(configRedeLocal.mqttBroker, configRedeLocal.mqttPort);
    mqttClient.setCallback(callbackMqtt);
    
    ultimaTentativaReconexaoMs = 0;
}

/**
 * Tenta conectar ao broker de forma assíncrona.
 */
static bool tentarConectarMqtt() {
    // Somente tenta conectar se o Wi-Fi estiver operacional
    if (WiFi.status() != WL_CONNECTED) {
        return false;
    }

    Serial.print(F("[MQTT] Tentando conectar ao broker: "));
    Serial.println(configRedeLocal.mqttBroker);

    // Usar o Client ID configurado via web (essencial para rodar múltiplos ESPs)
    const char* clientId = configRedeLocal.mqttClientId;

    bool conectado = false;
    if (strlen(configRedeLocal.mqttUser) > 0) {
        conectado = mqttClient.connect(clientId, configRedeLocal.mqttUser, configRedeLocal.mqttPassword);
    } else {
        conectado = mqttClient.connect(clientId);
    }

    if (conectado) {
        Serial.println(F("[MQTT] Conectado ao Broker!"));
        // Increver no tópico de comandos customizado (RF5.2)
        mqttClient.subscribe(configRedeLocal.mqttSubscribeTopic);
        Serial.print(F("[MQTT] Inscrito no tópico: "));
        Serial.println(configRedeLocal.mqttSubscribeTopic);
        return true;
    } else {
        Serial.print(F("[MQTT] Falha na conexao. Codigo de erro="));
        Serial.println(mqttClient.state());
        return false;
    }
}

void processarMqtt() {
    // Se o MQTT estiver desconectado, tenta reconectar sem bloquear o loop principal (ADR-010)
    if (!mqttClient.connected()) {
        unsigned long agora = millis();
        if (agora - ultimaTentativaReconexaoMs >= INTERVALO_RECONEXAO_MQTT_MS) {
            ultimaTentativaReconexaoMs = agora;
            tentarConectarMqtt();
        }
    } else {
        mqttClient.loop();
    }
}

void publicarTelemetria(bool valvulaLigada, float vazaoLMin, float volumeTotalL, const char* ultimaIrrigacao, const char* proximaIrrigacao) {
    if (!mqttClient.connected()) {
        return;
    }

    // Gerar JSON usando buffer estático em vez de String (evita fragmentação do Heap, conforme embedded-constraints.md)
    char payloadJson[256];
    snprintf(payloadJson, sizeof(payloadJson),
             "{\"valvula\":\"%s\",\"vazao\":%.2f,\"volume\":%.2f,\"ultima_irrigacao\":\"%s\",\"proxima_irrigacao\":\"%s\"}",
             valvulaLigada ? "LIGADA" : "DESLIGADA",
             vazaoLMin,
             volumeTotalL,
             ultimaIrrigacao,
             proximaIrrigacao);

    bool statusEnvio = mqttClient.publish(configRedeLocal.mqttPublishTopic, payloadJson);
    if (statusEnvio) {
        Serial.print(F("[MQTT] Telemetria publicada: "));
        Serial.println(payloadJson);
    } else {
        Serial.println(F("[MQTT] Erro ao publicar telemetria."));
    }
}

bool obterEstadoConexaoMqtt() {
    return mqttClient.connected();
}
