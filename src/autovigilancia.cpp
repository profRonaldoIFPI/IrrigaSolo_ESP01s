#include "autovigilancia.h"
#include "agendador.h"
#include "sensorFluxo.h"
#include <ESP8266WiFi.h>

// ---------------------------------------------------------------------------
// Variáveis internas de controle
// ---------------------------------------------------------------------------
static unsigned long ultimoMonitoramentoMs = 0;
static unsigned long ultimoWifiDesconectadoMs = 0;
static bool wifiEstavaConectado = true;

// Contador de leituras consecutivas de heap baixo (para evitar picos espúrios)
static uint8_t leiturasHeapBaixo = 0;

// ---------------------------------------------------------------------------
// Constantes de tempo e limites (conforme especificação de resiliência)
// ---------------------------------------------------------------------------
static const unsigned long INTERVALO_MONITORAMENTO_MS = 10000;   // Verificação a cada 10s
static const unsigned long TEMPO_MAX_DESCONECTADO_MS  = 3600000; // Limite de 1 hora desconectado em STA
static const unsigned long INTERVALO_RESET_DIARIO_MS  = 86400000; // Reset diário preventivo de 24 horas

// 6 KB é um limite que garante que o stack de rede e parser JSON funcionem com folga
static const uint32_t LIMITE_HEAP_CRITICO = 6000;

void inicializarAutovigilancia() {
    ultimoMonitoramentoMs = millis();
    ultimoWifiDesconectadoMs = millis();
    wifiEstavaConectado = (WiFi.status() == WL_CONNECTED);
    leiturasHeapBaixo = 0;

    Serial.println(F("[INFO] Autovigilância iniciada. Monitorando: Reset 24h, Heap > 6KB, WiFi STA Máx 1h."));
}

void processarAutovigilancia() {
    unsigned long agora = millis();

    // 1. Reinício Preventivo Agendado de 24 Horas
    // Se o uptime atingir o limite programado e a válvula solenoide estiver fechada
    if (agora >= INTERVALO_RESET_DIARIO_MS) {
        if (!obterEstadoRele(1)) {
            Serial.println(F("[WARN] Watchdog: Executando reinício preventivo programado de 24 horas."));
            forcarSalvamentoVolume();
            delay(500); // Aguarda buffer serial esvaziar
            ESP.restart();
        }
    }

    // 2. Verificações periódicas de saúde a cada 10 segundos (não-bloqueantes)
    if (agora - ultimoMonitoramentoMs >= INTERVALO_MONITORAMENTO_MS) {
        ultimoMonitoramentoMs = agora;

        // A) Monitoramento de Heap (Memory Leak / Heap Exhaustion)
        uint32_t heapLivre = ESP.getFreeHeap();
        if (heapLivre < LIMITE_HEAP_CRITICO) {
            leiturasHeapBaixo++;
            Serial.print(F("[WARN] Watchdog: Heap livre baixo detectado: "));
            Serial.print(heapLivre);
            Serial.print(F(" bytes. Amostra "));
            Serial.print(leiturasHeapBaixo);
            Serial.println(F("/3."));

            if (leiturasHeapBaixo >= 3) {
                // Heap crítico persistente! Reinicia de forma segura
                if (!obterEstadoRele(1)) {
                    Serial.println(F("[ERROR] Watchdog: Heap livre crítico persistente. Reiniciando dispositivo de forma segura."));
                    forcarSalvamentoVolume();
                    delay(500);
                    ESP.restart();
                } else {
                    Serial.println(F("[WARN] Watchdog: Heap crítico, mas a válvula está aberta. Aguardando fechamento para reiniciar."));
                }
            }
        } else {
            leiturasHeapBaixo = 0; // Normalizou
        }

        // B) Monitoramento de Conexão Wi-Fi Travada em STA
        // Monitoramos apenas se o chip estiver operando no modo cliente (WIFI_STA)
        if (WiFi.getMode() == WIFI_STA) {
            bool wifiConectado = (WiFi.status() == WL_CONNECTED);
            if (wifiConectado) {
                ultimoWifiDesconectadoMs = agora;
                wifiEstavaConectado = true;
            } else {
                if (wifiEstavaConectado) {
                    // Detectou a desconexão no instante atual
                    ultimoWifiDesconectadoMs = agora;
                    wifiEstavaConectado = false;
                    Serial.println(F("[WARN] Watchdog: Wi-Fi STA desconectado. Iniciando temporizador de tolerância de 1 hora."));
                } else {
                    // Monitora se o tempo desconectado contínuo ultrapassa o limite de 1 hora
                    if (agora - ultimoWifiDesconectadoMs >= TEMPO_MAX_DESCONECTADO_MS) {
                        if (!obterEstadoRele(1)) {
                            Serial.println(F("[ERROR] Watchdog: Wi-Fi STA desconectado há mais de 1 hora. Reiniciando rádio físico via reboot."));
                            forcarSalvamentoVolume();
                            delay(500);
                            ESP.restart();
                        }
                    }
                }
            }
        } else {
            // Se estiver em modo AP, reinicia os contadores (usuário configurando)
            ultimoWifiDesconectadoMs = agora;
            wifiEstavaConectado = true;
        }
    }
}
