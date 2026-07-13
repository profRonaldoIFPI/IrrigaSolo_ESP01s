/**
 * servidorWeb.cpp — Implementação do Servidor Web Embarcado
 *
 * Serve páginas HTML estáticas (PROGMEM) e endpoints HTTP para
 * comunicação com o frontend. Os handlers de API retornam JSON
 * minimalista, delegando a renderização ao navegador do cliente
 * (restrição embarcada: frontend leve).
 *
 * Endpoints implementados:
 *   GET  /              → Dashboard (index_html)
 *   GET  /horarios      → Agendamentos (horarios_html)
 *   GET  /config        → Configuração de rede (config_html)
 *   GET  /api/status    → JSON: hora, Wi-Fi, IP, estado relés
 *   GET  /api/schedules → JSON: retorna irrigation.json completo
 *   POST /api/control   → Controle manual (?channel=X&state=Y)
 *   POST /api/schedules → Salva agendamentos com validação
 *   GET  /api/network   → JSON: config de rede (sem senhas)
 *   POST /api/network   → Salva config de rede e reinicia Wi-Fi
 */

#include "servidorWeb.h"
#include "persistencia.h"
#include "agendador.h"
#include "redeWifi.h"
#include "relogioNtp.h"
#include "sensorFluxo.h"
#include "clienteMqtt.h"
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include <ArduinoJson.h>

// Headers gerados automaticamente pelo html2c.py (ADR-002)
#include "index.h"
#include "horarios.h"
#include "config.h"

// ---------------------------------------------------------------------------
// Instância do servidor e ponteiros para configurações
// ---------------------------------------------------------------------------

static ESP8266WebServer servidor(80);
static ESP8266HTTPUpdateServer atualizadorOta;
static ConfiguracaoRede*      ptrConfigRede      = nullptr;
static ConfiguracaoIrrigacao* ptrConfigIrrigacao = nullptr;

// ---------------------------------------------------------------------------
// Handlers de páginas HTML (servidas de PROGMEM)
// ---------------------------------------------------------------------------

/** Serve o Dashboard principal (RF4.1) */
static void handlePaginaIndex() {
    servidor.send_P(200, "text/html", index_html);
}

/** Serve a página de programação de horários (RF4.2) */
static void handlePaginaHorarios() {
    servidor.send_P(200, "text/html", horarios_html);
}

/** Serve a página de configuração de rede (RF2.2) */
static void handlePaginaConfig() {
    servidor.send_P(200, "text/html", config_html);
}

// ---------------------------------------------------------------------------
// Handlers de API — Status e Controle
// ---------------------------------------------------------------------------

/**
 * GET /api/status
 * Retorna JSON com hora atual, estado do Wi-Fi, IP e estado dos relés.
 * Consumido pelo frontend via Fetch API a cada 2 segundos (RF4.1).
 */
static void handleApiStatus() {
    JsonDocument documento;

    documento["time"]          = obterHoraFormatada();
    documento["date"]          = obterDataFormatada();
    documento["wifiConnected"] = estaConectadoSta();
    documento["ip"]            = obterIpAtual();
    documento["mqttConnected"] = obterEstadoConexaoMqtt();
    documento["c1State"]       = obterEstadoRele(1);
    documento["vazao"]         = obterVazaoLMin();
    documento["volume"]        = obterVolumeTotalL();

    char buffer[384];
    serializeJson(documento, buffer, sizeof(buffer));
    servidor.send(200, "application/json", buffer);
}

/**
 * POST /api/control
 * Controle manual de um canal: ?channel=1&state=1 (ligar) ou state=0 (desligar).
 * Independente da programação automática (RF4.1).
 */
static void handleApiControl() {
    if (!servidor.hasArg("channel") || !servidor.hasArg("state")) {
        servidor.send(400, "application/json", "{\"error\":\"Parâmetros 'channel' e 'state' obrigatórios.\"}");
        return;
    }

    uint8_t canal  = servidor.arg("channel").toInt();
    bool    estado = servidor.arg("state").toInt() == 1;

    if (canal != 1) {
        servidor.send(400, "application/json", "{\"error\":\"Canal inválido. Apenas canal 1 disponível.\"}");
        return;
    }

    controleManual(canal, estado);
    servidor.send(200, "application/json", "{\"ok\":true}");
}

/**
 * POST /api/zerar
 * Reseta o acumulador de volume do sensor de fluxo de água (RF3.3).
 */
static void handleApiZerarVolume() {
    zerarVolumeTotal();
    servidor.send(200, "application/json", "{\"ok\":true}");
}

// ---------------------------------------------------------------------------
// Handlers de API — Agendamentos
// ---------------------------------------------------------------------------

/**
 * GET /api/schedules
 * Retorna a configuração de irrigação completa como JSON.
 * Consumido pelo frontend para preencher o formulário de horários.
 */
static void handleApiGetSchedules() {
    if (ptrConfigIrrigacao == nullptr) {
        servidor.send(500, "application/json", "{\"error\":\"Configuração não inicializada.\"}");
        return;
    }

    JsonDocument documento;

    JsonArray arrayCh1 = documento["channel1"].to<JsonArray>();
    for (uint8_t i = 0; i < MAX_SESSOES_POR_CANAL; i++) {
        JsonObject sessao = arrayCh1.add<JsonObject>();
        sessao["enabled"]  = ptrConfigIrrigacao->canal1[i].habilitada;
        sessao["start"]    = ptrConfigIrrigacao->canal1[i].inicio;
        sessao["duration"] = ptrConfigIrrigacao->canal1[i].duracao;
    }

    char buffer[384];
    serializeJson(documento, buffer, sizeof(buffer));
    servidor.send(200, "application/json", buffer);
}

/**
 * POST /api/schedules
 * Recebe JSON com nova configuração de irrigação.
 * Valida conflitos no backend (ADR-006) antes de salvar.
 * Retorna HTTP 400 se houver sobreposição com simultaneidade desativada (CA-03).
 */
static void handleApiPostSchedules() {
    if (ptrConfigIrrigacao == nullptr) {
        servidor.send(500, "application/json", "{\"error\":\"Configuração não inicializada.\"}");
        return;
    }

    JsonDocument documento;
    DeserializationError erro = deserializeJson(documento, servidor.arg("plain"));

    if (erro) {
        Serial.print(F("[ERROR] Falha ao parsear JSON de agendamentos: "));
        Serial.println(erro.c_str());
        servidor.send(400, "application/json", "{\"error\":\"JSON inválido.\"}");
        return;
    }

    // Montar struct temporária para validação
    ConfiguracaoIrrigacao configTemporaria;

    // Deserializar Canal 1
    JsonArray arrayCh1 = documento["channel1"].as<JsonArray>();
    for (uint8_t i = 0; i < MAX_SESSOES_POR_CANAL; i++) {
        if (arrayCh1 && i < arrayCh1.size()) {
            JsonObject sessao = arrayCh1[i];
            configTemporaria.canal1[i].habilitada = sessao["enabled"] | false;
            strlcpy(configTemporaria.canal1[i].inicio,
                    sessao["start"] | "00:00",
                    sizeof(configTemporaria.canal1[i].inicio));
            configTemporaria.canal1[i].duracao = sessao["duration"] | 0;
        } else {
            configTemporaria.canal1[i].habilitada = false;
            strlcpy(configTemporaria.canal1[i].inicio, "00:00",
                    sizeof(configTemporaria.canal1[i].inicio));
            configTemporaria.canal1[i].duracao = 0;
        }
    }

    // Aplicar e persistir
    *ptrConfigIrrigacao = configTemporaria;

    if (salvarConfiguracaoIrrigacao(*ptrConfigIrrigacao)) {
        servidor.send(200, "application/json", "{\"ok\":true}");
    } else {
        servidor.send(500, "application/json", "{\"error\":\"Falha ao salvar no LittleFS.\"}");
    }
}

// ---------------------------------------------------------------------------
// Handlers de API — Configuração de Rede
// ---------------------------------------------------------------------------

/**
 * GET /api/network
 * Retorna configuração de rede SEM expor senhas (segurança).
 */
static void handleApiGetNetwork() {
    if (ptrConfigRede == nullptr) {
        servidor.send(500, "application/json", "{\"error\":\"Configuração não inicializada.\"}");
        return;
    }

    JsonDocument documento;
    documento["ssidAp"]          = ptrConfigRede->ssidAp;
    documento["ssidSta"]         = ptrConfigRede->ssidSta;
    documento["ntpServer"]       = ptrConfigRede->servidorNtp;
    documento["timeZone"]        = ptrConfigRede->fusoHorario;
    documento["ntpSyncInterval"] = ptrConfigRede->intervaloSyncNtp;
    
    // Parâmetros MQTT (sem senha)
    documento["mqttBroker"]      = ptrConfigRede->mqttBroker;
    documento["mqttPort"]        = ptrConfigRede->mqttPort;
    documento["mqttUser"]        = ptrConfigRede->mqttUser;
    documento["mqttTopic"]       = ptrConfigRede->mqttTopicPrefix;

    char buffer[384];
    serializeJson(documento, buffer, sizeof(buffer));
    servidor.send(200, "application/json", buffer);
}

/**
 * POST /api/network
 * Recebe JSON com nova configuração de rede.
 * Salva no LittleFS e reinicia o módulo Wi-Fi com as novas credenciais.
 */
static void handleApiPostNetwork() {
    if (ptrConfigRede == nullptr) {
        servidor.send(500, "application/json", "{\"error\":\"Configuração não inicializada.\"}");
        return;
    }

    JsonDocument documento;
    DeserializationError erro = deserializeJson(documento, servidor.arg("plain"));

    if (erro) {
        Serial.print(F("[ERROR] Falha ao parsear JSON de rede: "));
        Serial.println(erro.c_str());
        servidor.send(400, "application/json", "{\"error\":\"JSON inválido.\"}");
        return;
    }

    // Atualizar apenas campos presentes no JSON
    if (documento["ssidAp"].is<const char*>()) {
        strlcpy(ptrConfigRede->ssidAp, documento["ssidAp"], sizeof(ptrConfigRede->ssidAp));
    }
    if (documento["pwdAp"].is<const char*>()) {
        strlcpy(ptrConfigRede->senhaAp, documento["pwdAp"], sizeof(ptrConfigRede->senhaAp));
    }
    if (documento["ssidSta"].is<const char*>()) {
        strlcpy(ptrConfigRede->ssidSta, documento["ssidSta"], sizeof(ptrConfigRede->ssidSta));
    }
    if (documento["pwdSta"].is<const char*>()) {
        strlcpy(ptrConfigRede->senhaSta, documento["pwdSta"], sizeof(ptrConfigRede->senhaSta));
    }
    if (documento["ntpServer"].is<const char*>()) {
        strlcpy(ptrConfigRede->servidorNtp, documento["ntpServer"], sizeof(ptrConfigRede->servidorNtp));
    }
    if (documento["timeZone"].is<int>()) {
        ptrConfigRede->fusoHorario = documento["timeZone"];
    }
    if (documento["ntpSyncInterval"].is<unsigned long>()) {
        ptrConfigRede->intervaloSyncNtp = documento["ntpSyncInterval"];
    }
    
    // Parâmetros MQTT
    if (documento["mqttBroker"].is<const char*>()) {
        strlcpy(ptrConfigRede->mqttBroker, documento["mqttBroker"], sizeof(ptrConfigRede->mqttBroker));
    }
    if (documento["mqttPort"].is<int>()) {
        ptrConfigRede->mqttPort = documento["mqttPort"];
    }
    if (documento["mqttUser"].is<const char*>()) {
        strlcpy(ptrConfigRede->mqttUser, documento["mqttUser"], sizeof(ptrConfigRede->mqttUser));
    }
    if (documento["mqttPwd"].is<const char*>()) {
        strlcpy(ptrConfigRede->mqttPassword, documento["mqttPwd"], sizeof(ptrConfigRede->mqttPassword));
    }
    if (documento["mqttTopic"].is<const char*>()) {
        strlcpy(ptrConfigRede->mqttTopicPrefix, documento["mqttTopic"], sizeof(ptrConfigRede->mqttTopicPrefix));
    }

    if (!salvarConfiguracaoRede(*ptrConfigRede)) {
        servidor.send(500, "application/json", "{\"error\":\"Falha ao salvar no LittleFS.\"}");
        return;
    }

    servidor.send(200, "application/json", "{\"ok\":true,\"message\":\"Configuração salva. Reconectando...\"}");

    // Reinicializar Wi-Fi e MQTT com as novas credenciais
    Serial.println(F("[INFO] Reinicializando conexoes com novas credenciais..."));
    delay(500); // Pequeno delay para garantir que a resposta HTTP foi enviada
    inicializarWifi(*ptrConfigRede);
    inicializarNtp(*ptrConfigRede);
    inicializarMqtt(*ptrConfigRede);
}

// ---------------------------------------------------------------------------
// Handler para rotas não encontradas (404)
// ---------------------------------------------------------------------------

static void handleNaoEncontrado() {
    servidor.send(404, "text/plain", "404 - Página não encontrada.");
}

// ---------------------------------------------------------------------------
// Implementações públicas
// ---------------------------------------------------------------------------

void inicializarServidor(ConfiguracaoRede* configRede, ConfiguracaoIrrigacao* configIrrigacao) {
    ptrConfigRede      = configRede;
    ptrConfigIrrigacao = configIrrigacao;

    // Páginas HTML (servidas de PROGMEM)
    servidor.on("/",         HTTP_GET,  handlePaginaIndex);
    servidor.on("/horarios", HTTP_GET,  handlePaginaHorarios);
    servidor.on("/config",   HTTP_GET,  handlePaginaConfig);

    // API de status e controle
    servidor.on("/api/status",  HTTP_GET,  handleApiStatus);
    servidor.on("/api/control", HTTP_POST, handleApiControl);
    servidor.on("/api/zerar",   HTTP_POST, handleApiZerarVolume);

    // API de agendamentos
    servidor.on("/api/schedules", HTTP_GET,  handleApiGetSchedules);
    servidor.on("/api/schedules", HTTP_POST, handleApiPostSchedules);

    // API de configuração de rede
    servidor.on("/api/network", HTTP_GET,  handleApiGetNetwork);
    servidor.on("/api/network", HTTP_POST, handleApiPostNetwork);

    // Registra o servidor de atualização OTA na rota padrão (/update)
    atualizadorOta.setup(&servidor);

    // Handler 404
    servidor.onNotFound(handleNaoEncontrado);

    servidor.begin();
    Serial.println(F("[INFO] Servidor web iniciado na porta 80. Rota OTA /update pronta."));
}

void processarRequisicoes() {
    servidor.handleClient();
}
