/**
 * persistencia.cpp — Implementação do Módulo de Persistência LittleFS
 *
 * Gerencia a leitura e escrita dos arquivos de configuração JSON.
 * Utiliza ArduinoJson v7 com JsonDocument de tamanho calculado
 * para evitar alocação dinâmica desnecessária no Heap.
 *
 * Estratégia de segurança (RF5.1, RF5.2):
 *   - Arquivo ausente → aplica defaults seguros.
 *   - Arquivo corrompido (JSON inválido) → aplica defaults seguros,
 *     registra detalhadamente o erro via Serial (ADR-008).
 *   - Falha de escrita → retorna false, registra erro.
 */

#include "persistencia.h"
#include <LittleFS.h>
#include <ArduinoJson.h>

// ---------------------------------------------------------------------------
// Caminhos dos arquivos no LittleFS (ADR-004)
// ---------------------------------------------------------------------------
static const char CAMINHO_NETWORK[]    = "/network.json";
static const char CAMINHO_IRRIGATION[] = "/irrigation.json";

// ---------------------------------------------------------------------------
// Funções auxiliares internas
// ---------------------------------------------------------------------------

/**
 * Aplica os valores padrão de fábrica na struct de rede.
 * Usado quando /network.json está ausente ou corrompido (RF5.1).
 * Garante que o sistema entre em modo AP com credenciais previsíveis.
 */
static void aplicarDefaultsRede(ConfiguracaoRede& configRede) {
    strlcpy(configRede.ssidAp,       SSID_AP_PADRAO,       sizeof(configRede.ssidAp));
    strlcpy(configRede.senhaAp,      SENHA_AP_PADRAO,      sizeof(configRede.senhaAp));
    
    // Wi-Fi Local (IoTnet)
    strlcpy(configRede.ssidSta,      "IoTnet",             sizeof(configRede.ssidSta));
    strlcpy(configRede.senhaSta,     "Casa20@iot_devices", sizeof(configRede.senhaSta));
    
    strlcpy(configRede.servidorNtp,   SERVIDOR_NTP_PADRAO,  sizeof(configRede.servidorNtp));
    configRede.fusoHorario       = FUSO_HORARIO_PADRAO;
    configRede.intervaloSyncNtp  = INTERVALO_NTP_PADRAO;

    // Padrões MQTT para Broker Mosquitto local
    strlcpy(configRede.mqttBroker, "192.168.1.49", sizeof(configRede.mqttBroker));
    configRede.mqttPort = 1883;
    strlcpy(configRede.mqttUser, "iotdevices", sizeof(configRede.mqttUser));
    strlcpy(configRede.mqttPassword, "iotCasa20", sizeof(configRede.mqttPassword));
    strlcpy(configRede.mqttClientId, "irrigasolo_1", sizeof(configRede.mqttClientId));
    strlcpy(configRede.mqttPublishTopic, "irriga_solo/1/status", sizeof(configRede.mqttPublishTopic));
    strlcpy(configRede.mqttSubscribeTopic, "irriga_solo/1/comando", sizeof(configRede.mqttSubscribeTopic));
}

/**
 * Aplica os valores padrão seguros na struct de irrigação.
 * Todas as sessões ficam desabilitadas, impedindo acionamento
 * automático do relé (RF5.2, CA-01).
 */
static void aplicarDefaultsIrrigacao(ConfiguracaoIrrigacao& configIrrigacao) {
    for (uint8_t i = 0; i < MAX_SESSOES_POR_CANAL; i++) {
        configIrrigacao.canal1[i].habilitada = false;
        strlcpy(configIrrigacao.canal1[i].inicio, "00:00", sizeof(configIrrigacao.canal1[i].inicio));
        configIrrigacao.canal1[i].duracao = 0;
    }
}

// ---------------------------------------------------------------------------
// Implementações públicas
// ---------------------------------------------------------------------------

bool inicializarLittleFS() {
    if (!LittleFS.begin()) {
        Serial.println(F("[ERROR] Falha ao montar o LittleFS. O sistema de arquivos pode estar corrompido."));
        return false;
    }
    Serial.println(F("[INFO] LittleFS montado com sucesso."));
    return true;
}

bool carregarConfiguracaoRede(ConfiguracaoRede& configRede) {
    if (!LittleFS.exists(CAMINHO_NETWORK)) {
        Serial.println(F("[WARN] /network.json não encontrado. Aplicando defaults de fábrica (modo AP)."));
        aplicarDefaultsRede(configRede);
        return false;
    }

    File arquivo = LittleFS.open(CAMINHO_NETWORK, "r");
    if (!arquivo) {
        Serial.println(F("[ERROR] Falha ao abrir /network.json para leitura."));
        aplicarDefaultsRede(configRede);
        return false;
    }

    // JsonDocument calcula o tamanho necessário automaticamente no ArduinoJson v7
    JsonDocument documento;
    DeserializationError erro = deserializeJson(documento, arquivo);
    arquivo.close();

    if (erro) {
        // ADR-008: registrar detalhadamente o nó que falhou
        Serial.print(F("[ERROR] Falha ao parsear /network.json: "));
        Serial.println(erro.c_str());
        aplicarDefaultsRede(configRede);
        return false;
    }

    // Extrai campos com fallback para defaults caso algum campo esteja ausente
    strlcpy(configRede.ssidAp,
            documento["ssidAp"]  | SSID_AP_PADRAO,
            sizeof(configRede.ssidAp));
    strlcpy(configRede.senhaAp,
            documento["pwdAp"]   | SENHA_AP_PADRAO,
            sizeof(configRede.senhaAp));
    strlcpy(configRede.ssidSta,
            documento["ssidSta"] | "",
            sizeof(configRede.ssidSta));
    strlcpy(configRede.senhaSta,
            documento["pwdSta"]  | "",
            sizeof(configRede.senhaSta));
    strlcpy(configRede.servidorNtp,
            documento["ntpServer"] | SERVIDOR_NTP_PADRAO,
            sizeof(configRede.servidorNtp));

    configRede.fusoHorario      = documento["timeZone"]        | FUSO_HORARIO_PADRAO;
    configRede.intervaloSyncNtp = documento["ntpSyncInterval"] | INTERVALO_NTP_PADRAO;

    // Extrair parâmetros MQTT
    strlcpy(configRede.mqttBroker,
            documento["mqttBroker"] | "192.168.1.49",
            sizeof(configRede.mqttBroker));
    configRede.mqttPort = documento["mqttPort"] | 1883;
    strlcpy(configRede.mqttUser,
            documento["mqttUser"] | "iotdevices",
            sizeof(configRede.mqttUser));
    strlcpy(configRede.mqttPassword,
            documento["mqttPwd"] | "iotCasa20",
            sizeof(configRede.mqttPassword));
    strlcpy(configRede.mqttClientId,
            documento["mqttClientId"] | "irrigasolo_1",
            sizeof(configRede.mqttClientId));
    strlcpy(configRede.mqttPublishTopic,
            documento["mqttPubTopic"] | "irriga_solo/1/status",
            sizeof(configRede.mqttPublishTopic));
    strlcpy(configRede.mqttSubscribeTopic,
            documento["mqttSubTopic"] | "irriga_solo/1/comando",
            sizeof(configRede.mqttSubscribeTopic));

    Serial.println(F("[INFO] /network.json carregado com sucesso."));
    return true;
}

bool salvarConfiguracaoRede(const ConfiguracaoRede& configRede) {
    JsonDocument documento;

    documento["ssidAp"]          = configRede.ssidAp;
    documento["pwdAp"]           = configRede.senhaAp;
    documento["ssidSta"]         = configRede.ssidSta;
    documento["pwdSta"]          = configRede.senhaSta;
    documento["ntpServer"]       = configRede.servidorNtp;
    documento["timeZone"]        = configRede.fusoHorario;
    documento["ntpSyncInterval"] = configRede.intervaloSyncNtp;

    // Campos MQTT
    documento["mqttBroker"]      = configRede.mqttBroker;
    documento["mqttPort"]        = configRede.mqttPort;
    documento["mqttUser"]        = configRede.mqttUser;
    documento["mqttPwd"]         = configRede.mqttPassword;
    documento["mqttClientId"]    = configRede.mqttClientId;
    documento["mqttPubTopic"]    = configRede.mqttPublishTopic;
    documento["mqttSubTopic"]    = configRede.mqttSubscribeTopic;

    File arquivo = LittleFS.open(CAMINHO_NETWORK, "w");
    if (!arquivo) {
        Serial.println(F("[ERROR] Falha ao abrir /network.json para escrita."));
        return false;
    }

    size_t bytesEscritos = serializeJson(documento, arquivo);
    arquivo.close();

    if (bytesEscritos == 0) {
        Serial.println(F("[ERROR] Falha ao serializar /network.json (0 bytes escritos)."));
        return false;
    }

    Serial.println(F("[INFO] /network.json salvo com sucesso."));
    return true;
}

bool carregarConfiguracaoIrrigacao(ConfiguracaoIrrigacao& configIrrigacao) {
    if (!LittleFS.exists(CAMINHO_IRRIGATION)) {
        Serial.println(F("[WARN] /irrigation.json não encontrado. Aplicando defaults seguros (relés inativos)."));
        aplicarDefaultsIrrigacao(configIrrigacao);
        return false;
    }

    File arquivo = LittleFS.open(CAMINHO_IRRIGATION, "r");
    if (!arquivo) {
        Serial.println(F("[ERROR] Falha ao abrir /irrigation.json para leitura."));
        aplicarDefaultsIrrigacao(configIrrigacao);
        return false;
    }

    JsonDocument documento;
    DeserializationError erro = deserializeJson(documento, arquivo);
    arquivo.close();

    if (erro) {
        Serial.print(F("[ERROR] Falha ao parsear /irrigation.json: "));
        Serial.println(erro.c_str());
        aplicarDefaultsIrrigacao(configIrrigacao);
        return false;
    }

    // Deserializar sessões do Canal 1
    JsonArray arrayCh1 = documento["channel1"].as<JsonArray>();
    for (uint8_t i = 0; i < MAX_SESSOES_POR_CANAL; i++) {
        if (arrayCh1 && i < arrayCh1.size()) {
            JsonObject sessao = arrayCh1[i];
            configIrrigacao.canal1[i].habilitada = sessao["enabled"] | false;
            strlcpy(configIrrigacao.canal1[i].inicio,
                    sessao["start"] | "00:00",
                    sizeof(configIrrigacao.canal1[i].inicio));
            configIrrigacao.canal1[i].duracao = sessao["duration"] | 0;
        } else {
            // Slots não definidos no JSON ficam desabilitados
            configIrrigacao.canal1[i].habilitada = false;
            strlcpy(configIrrigacao.canal1[i].inicio, "00:00",
                    sizeof(configIrrigacao.canal1[i].inicio));
            configIrrigacao.canal1[i].duracao = 0;
        }
        yield(); // Alimentar o WDT durante parsing de arrays (restrição embarcada)
    }

    Serial.println(F("[INFO] /irrigation.json carregado com sucesso."));
    return true;
}

bool salvarConfiguracaoIrrigacao(const ConfiguracaoIrrigacao& configIrrigacao) {
    JsonDocument documento;

    // Serializar Canal 1
    JsonArray arrayCh1 = documento["channel1"].to<JsonArray>();
    for (uint8_t i = 0; i < MAX_SESSOES_POR_CANAL; i++) {
        JsonObject sessao = arrayCh1.add<JsonObject>();
        sessao["enabled"]  = configIrrigacao.canal1[i].habilitada;
        sessao["start"]    = configIrrigacao.canal1[i].inicio;
        sessao["duration"] = configIrrigacao.canal1[i].duracao;
    }

    File arquivo = LittleFS.open(CAMINHO_IRRIGATION, "w");
    if (!arquivo) {
        Serial.println(F("[ERROR] Falha ao abrir /irrigation.json para escrita."));
        return false;
    }

    size_t bytesEscritos = serializeJson(documento, arquivo);
    arquivo.close();

    if (bytesEscritos == 0) {
        Serial.println(F("[ERROR] Falha ao serializar /irrigation.json (0 bytes escritos)."));
        return false;
    }

    Serial.println(F("[INFO] /irrigation.json salvo com sucesso."));
    return true;
}
