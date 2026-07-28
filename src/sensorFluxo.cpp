#include "sensorFluxo.h"
#include "pinosHardware.h"
#include "agendador.h"
#include "persistencia.h"

// Variáveis locais para controle de salvamento na Flash
static float ultimoVolumeSalvoL = 0.0f;
static bool valvulaEstavaLigada = false;

// Contador de pulsos incrementado na interrupção de hardware (deve ser volatile)
static volatile uint32_t contagemPulsos = 0;

// Variáveis de controle de tempo e cálculo
static unsigned long ultimoCalculoMs = 0;
static float vazaoLMin = 0.0f;
static float volumeTotalL = 0.0f;

/**
 * Rotina de Serviço de Interrupção (ISR) executada em memória RAM (IRAM_ATTR).
 * Chamada a cada borda de subida do sensor Hall.
 */
void IRAM_ATTR tratarPulsoSensor() {
    contagemPulsos++;
}

void inicializarSensorFluxo() {
    // Usar pullup interno para garantir nível lógico estável no sinal
    pinMode(PINO_SENSOR_FLUXO, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(PINO_SENSOR_FLUXO), tratarPulsoSensor, RISING);
    
    ultimoCalculoMs = millis();
    contagemPulsos = 0;
    vazaoLMin = 0.0f;
    
    // Carregar volume acumulado salvo da Flash (persistência)
    if (!carregarVolumeAcumulado(volumeTotalL)) {
        volumeTotalL = 0.0f;
    }
    ultimoVolumeSalvoL = volumeTotalL;
    valvulaEstavaLigada = obterEstadoRele(1);
    
    Serial.print(F("[INFO] Sensor de fluxo YF-S201B inicializado. Volume acumulado inicial: "));
    Serial.print(volumeTotalL);
    Serial.println(F(" L"));
}

void processarSensorFluxo() {
    unsigned long agora = millis();
    
    // Executa a lógica de integração a cada 1 segundo (1000 ms)
    if (agora - ultimoCalculoMs >= 1000) {
        // Desabilitar interrupções temporariamente para leitura atômica do contador
        noInterrupts();
        uint32_t pulsosNoIntervalo = contagemPulsos;
        contagemPulsos = 0;
        interrupts();

        unsigned long intervaloMs = agora - ultimoCalculoMs;
        ultimoCalculoMs = agora;

        // Frequência de pulsos (Hz)
        float frequenciaHz = (float)pulsosNoIntervalo / ((float)intervaloMs / 1000.0f);

        // Vazão em L/min = Hz / 7.5 (FATOR_CALIBRACAO_FLUXO)
        vazaoLMin = frequenciaHz / FATOR_CALIBRACAO_FLUXO;

        // Volume no intervalo (L) = Vazão (L/min) * Tempo (minutos)
        // Tempo em minutos = intervaloMs / 60000.0f
        float volumeIntervaloL = vazaoLMin * ((float)intervaloMs / 60000.0f);
        volumeTotalL += volumeIntervaloL;

        // Lógica de gravação inteligente no LittleFS para evitar desgaste da Flash (RULE[embedded-constraints.md])
        bool valvulaLigada = obterEstadoRele(1);

        // Cenário 1: Válvula fechou (transição de ligada para desligada)
        if (valvulaEstavaLigada && !valvulaLigada) {
            if (volumeTotalL != ultimoVolumeSalvoL) {
                Serial.println(F("[INFO] Válvula fechada. Salvando volume acumulado."));
                salvarVolumeAcumulado(volumeTotalL);
                ultimoVolumeSalvoL = volumeTotalL;
            }
        }
        // Cenário 2: Acúmulo de mais de 5.0 Litros desde a última gravação (segurança contra quedas de energia)
        else if (volumeTotalL - ultimoVolumeSalvoL >= 5.0f) {
            Serial.println(F("[INFO] Acúmulo de 5L desde a última gravação. Salvando volume acumulado preventivamente."));
            salvarVolumeAcumulado(volumeTotalL);
            ultimoVolumeSalvoL = volumeTotalL;
        }

        valvulaEstavaLigada = valvulaLigada;
    }
}

float obterVazaoLMin() {
    return vazaoLMin;
}

float obterVolumeTotalL() {
    return volumeTotalL;
}

void zerarVolumeTotal() {
    noInterrupts();
    volumeTotalL = 0.0f;
    interrupts();
    ultimoVolumeSalvoL = 0.0f;
    salvarVolumeAcumulado(0.0f); // Persiste a zeragem na Flash imediatamente
    Serial.println(F("[INFO] Volume total de agua zerado manualmente e salvo na Flash."));
}

void forcarSalvamentoVolume() {
    if (volumeTotalL != ultimoVolumeSalvoL) {
        Serial.println(F("[INFO] Forçando salvamento de volume acumulado antes do reinício."));
        salvarVolumeAcumulado(volumeTotalL);
        ultimoVolumeSalvoL = volumeTotalL;
    }
}
