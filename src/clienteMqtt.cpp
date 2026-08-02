#include "clienteMqtt.h"
#include "agendador.h"
#include "sensorFluxo.h"
#include "autovigilancia.h"
#include "persistencia.h"
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// Instâncias globais locais do módulo
static WiFiClient espClient;
static PubSubClient mqttClient(espClient);

static ConfiguracaoRede configRedeLocal;
static ConfiguracaoIrrigacao* ponteiroConfigIrrigacao = nullptr;

static unsigned long ultimaTentativaReconexaoMs = 0;
static const unsigned long INTERVALO_RECONEXAO_MQTT_MS = 10000;

// A configuração MQTT agora lê os tópicos e Client ID dinamicamente do LittleFS

void registrarConfiguracaoIrrigacaoMqtt(ConfiguracaoIrrigacao* config) {
    ponteiroConfigIrrigacao = config;
}

void publicarStatusReboot() {
    if (!mqttClient.connected()) {
        return;
    }

    char topicoReboot[128];
    snprintf(topicoReboot, sizeof(topicoReboot), "%s/reboot", configRedeLocal.mqttPublishTopic);

    char payloadJson[256];
    snprintf(payloadJson, sizeof(payloadJson),
             "{\"id\":\"%s\",\"motivo_reset\":\"%s\",\"uptime\":%u,\"disparos_espurios\":%u}",
             configRedeLocal.mqttClientId,
             obterMotivoReset(),
             (uint32_t)(millis() / 1000),
             obterContadorDisparosEspurios());

    bool status = mqttClient.publish(topicoReboot, payloadJson);
    if (status) {
        Serial.print(F("[MQTT] Status de Reboot publicado: "));
        Serial.println(payloadJson);
    }
}

void publicarAgendamentos() {
    if (!mqttClient.connected() || ponteiroConfigIrrigacao == nullptr) {
        return;
    }

    char topicoAgendamentos[128];
    snprintf(topicoAgendamentos, sizeof(topicoAgendamentos), "%s/agendamentos", configRedeLocal.mqttPublishTopic);

    char payloadJson[512];
    int pos = snprintf(payloadJson, sizeof(payloadJson), "{\"id\":\"%s\",\"sessoes\":[", configRedeLocal.mqttClientId);

    for (uint8_t i = 0; i < MAX_SESSOES_POR_CANAL; i++) {
        const SessaoIrrigacao& s = ponteiroConfigIrrigacao->canal1[i];
        pos += snprintf(payloadJson + pos, sizeof(payloadJson) - pos,
                        "{\"habilitada\":%s,\"inicio\":\"%s\",\"duracao\":%d}%s",
                        s.habilitada ? "true" : "false",
                        s.inicio,
                        s.duracao,
                        (i < MAX_SESSOES_POR_CANAL - 1) ? "," : "");
    }
    snprintf(payloadJson + pos, sizeof(payloadJson) - pos, "]}");

    bool status = mqttClient.publish(topicoAgendamentos, payloadJson);
    if (status) {
        Serial.print(F("[MQTT] Agendamentos publicados: "));
        Serial.println(payloadJson);
    }
}

/**
 * Função de callback interna. Chamada pela PubSubClient quando chega
 * uma mensagem nos tópicos inscritos (ex: comando).
 */
static void callbackMqtt(char* topic, byte* payload, unsigned int length) {
    // Converter o payload binário para string com segurança (buffer expandido para suportar JSON)
    char payloadStr[256];
    unsigned int tamanhoCopiar = length < (sizeof(payloadStr) - 1) ? length : (sizeof(payloadStr) - 1);
    memcpy(payloadStr, payload, tamanhoCopiar);
    payloadStr[tamanhoCopiar] = '\0';

    Serial.print(F("[MQTT] Mensagem recebida no tópico ["));
    Serial.print(topic);
    Serial.print(F("]: "));
    Serial.println(payloadStr);

    // 1. Processar payload se for um JSON de agendamento de irrigação (RF5.2)
    if (payloadStr[0] == '{') {
        JsonDocument doc;
        DeserializationError erro = deserializeJson(doc, payloadStr);
        if (!erro) {
            if (doc["inicio"].is<const char*>() && doc["duracao"].is<int>()) {
                uint8_t sessaoIndex = doc["sessao"] | 0;
                if (sessaoIndex < MAX_SESSOES_POR_CANAL && ponteiroConfigIrrigacao != nullptr) {
                    SessaoIrrigacao& s = ponteiroConfigIrrigacao->canal1[sessaoIndex];
                    s.habilitada = doc["habilitada"] | true;
                    strlcpy(s.inicio, doc["inicio"].as<const char*>(), sizeof(s.inicio));
                    s.duracao = (uint8_t)doc["duracao"].as<int>();

                    // Persistir a nova configuração no LittleFS (/irrigation.json)
                    salvarConfiguracaoIrrigacao(*ponteiroConfigIrrigacao);

                    Serial.print(F("[MQTT] Agendamento atualizado via JSON para sessão "));
                    Serial.print(sessaoIndex);
                    Serial.println(F(" e salvo no LittleFS!"));

                    // Confirmar enviando a lista atualizada de agendamentos
                    publicarAgendamentos();
                    return;
                }
            }
        } else {
            Serial.print(F("[MQTT] Erro ao decodificar JSON de agendamento: "));
            Serial.println(erro.c_str());
        }
    }

    // 2. Processamento de comandos de texto legíveis (RF5.2)
    if (strcmp(payloadStr, "LIGAR") == 0) {
        controleManual(1, true);
    } else if (strcmp(payloadStr, "DESLIGAR") == 0) {
        controleManual(1, false);
    } else if (strcmp(payloadStr, "ZERAR") == 0) {
        zerarVolumeTotal();
    } else if (strcmp(payloadStr, "AGENDAMENTOS") == 0 || strcmp(payloadStr, "OBTER_AGENDAMENTOS") == 0) {
        publicarAgendamentos();
    }
}

void inicializarMqtt(const ConfiguracaoRede& config, ConfiguracaoIrrigacao* configIrrigacao) {
    configRedeLocal = config;
    ponteiroConfigIrrigacao = configIrrigacao;

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
        // Inscrever no tópico de comandos customizado (RF5.2)
        mqttClient.subscribe(configRedeLocal.mqttSubscribeTopic);
        Serial.print(F("[MQTT] Inscrito no tópico de comandos: "));
        Serial.println(configRedeLocal.mqttSubscribeTopic);

        // Inscrever no tópico de agendamento dedicado <mqttSubTopic>/agendar
        char topicoInscricaoAgendar[128];
        snprintf(topicoInscricaoAgendar, sizeof(topicoInscricaoAgendar), "%s/agendar", configRedeLocal.mqttSubscribeTopic);
        mqttClient.subscribe(topicoInscricaoAgendar);
        Serial.print(F("[MQTT] Inscrito no tópico de agendar: "));
        Serial.println(topicoInscricaoAgendar);

        // Publicar nos tópicos dedicados de reboot e agendamentos (RF5.3, CA-09)
        publicarStatusReboot();
        publicarAgendamentos();

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

    // Coleta de dados de diagnóstico em tempo real
    IPAddress ip = WiFi.localIP();
    int32_t rssi = WiFi.RSSI();
    uint32_t heapLivre = ESP.getFreeHeap();
    uint32_t tempoOnline = millis() / 1000;
    uint32_t chipId = ESP.getChipId();
    uint32_t disparosEspurios = obterContadorDisparosEspurios();

    // Buffer estático expandido para evitar fragmentação e suportar a carga de telemetria expandida (embedded-constraints.md)
    char payloadJson[512];
    snprintf(payloadJson, sizeof(payloadJson),
             "{\"id\":\"%s\",\"chip_id\":%u,\"ip\":\"%d.%d.%d.%d\",\"valvula\":\"%s\",\"vazao\":%.2f,\"volume\":%.2f,\"ultima_irrigacao\":\"%s\",\"proxima_irrigacao\":\"%s\",\"uptime\":%u,\"rssi\":%d,\"heap_livre\":%u,\"motivo_reset\":\"%s\",\"disparos_espurios\":%u}",
             configRedeLocal.mqttClientId,
             chipId,
             ip[0], ip[1], ip[2], ip[3],
             valvulaLigada ? "LIGADA" : "DESLIGADA",
             vazaoLMin,
             volumeTotalL,
             ultimaIrrigacao,
             proximaIrrigacao,
             tempoOnline,
             rssi,
             heapLivre,
             obterMotivoReset(),
             disparosEspurios);

    bool statusEnvio = mqttClient.publish(configRedeLocal.mqttPublishTopic, payloadJson);
    if (statusEnvio) {
        Serial.print(F("[MQTT] Telemetria publicada com sucesso: "));
        Serial.println(payloadJson);
    } else {
        Serial.println(F("[MQTT] Erro ao publicar telemetria."));
    }
}

bool obterEstadoConexaoMqtt() {
    return mqttClient.connected();
}
