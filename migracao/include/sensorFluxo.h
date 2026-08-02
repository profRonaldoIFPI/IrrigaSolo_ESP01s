#ifndef SENSOR_FLUXO_IDF_H
#define SENSOR_FLUXO_IDF_H

#include <stdint.h>

/**
 * Configura o pino de interrupção GPIO do sensor de fluxo usando o driver do ESP32 (gpio_isr_handler_add).
 */
void inicializarSensorFluxoIdf(void);

/**
 * Calcula a vazão instantânea e acumula o volume a cada segundo.
 */
void processarSensorFluxoIdf(void);

/**
 * Retorna a vazão em L/min.
 */
float obterVazaoLMinIdf(void);

/**
 * Retorna o volume total acumulado em Litros.
 */
float obterVolumeTotalLIdf(void);

/**
 * Zera o contador de volume acumulado.
 */
void zerarVolumeTotalIdf(void);

#endif // SENSOR_FLUXO_IDF_H
