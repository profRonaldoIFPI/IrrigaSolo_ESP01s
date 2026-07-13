#include "sensorFluxo.h"
#include "pinosHardware.h"

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
    volumeTotalL = 0.0f;
    
    Serial.println(F("[INFO] Sensor de fluxo YF-S201B inicializado com sucesso."));
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
    Serial.println(F("[INFO] Volume total de agua zerado manualmente."));
}
