#ifndef PINOS_HARDWARE_IDF_H
#ifndef PINOS_HARDWARE_IDF_H
#define PINOS_HARDWARE_IDF_H

/**
 * pinosHardware.h — Constantes de Hardware para ESP-IDF (ESP32)
 *
 * Mapeia os pino GPIO utilizando os tipos nativos gpio_num_t do ESP-IDF.
 */

#include "driver/gpio.h"
#include <stdint.h>
#include <stdbool.h>

// GPIOs do ESP32
static const gpio_num_t PINO_RELE_CANAL_1 = GPIO_NUM_5;   // GPIO 5
static const gpio_num_t PINO_SENSOR_FLUXO  = GPIO_NUM_14;  // GPIO 14
static const gpio_num_t PINO_LED_STATUS   = GPIO_NUM_2;   // GPIO 2 (LED Onboard ESP32)

static const float FATOR_CALIBRACAO_FLUXO = 7.5f;
static const uint8_t MAX_SESSOES_POR_CANAL = 6;

static const char VERSAO_FIRMWARE_IDF[] = "v1.2-IDF";

#endif // PINOS_HARDWARE_IDF_H
