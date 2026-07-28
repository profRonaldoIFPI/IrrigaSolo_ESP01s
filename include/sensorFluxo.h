#pragma once

/**
 * sensorFluxo.h — Interface de leitura do sensor de fluxo YF-S201B
 *
 * Este módulo gerencia a leitura física e o cálculo matemático do fluxo de
 * água. Os pulsos gerados pelo sensor Hall são contados através de uma 
 * interrupção externa (ISR) no pino configurado.
 *
 * Idioma: Português
 * Convenção de nomenclatura: camelCase para funções e variáveis.
 */

#include <Arduino.h>

/**
 * Configura o pino do sensor de fluxo como entrada e anexa a interrupção externa
 * para detectar a borda de subida (RISING) dos pulsos do Hall.
 */
void inicializarSensorFluxo();

/**
 * Executa o cálculo da vazão (L/min) e acumulação de volume (Litros) a cada segundo.
 * Deve ser chamada no loop principal do firmware de forma não-bloqueante.
 */
void processarSensorFluxo();

/**
 * Retorna a vazão instantânea calculada.
 *
 * @return Vazão instantânea em Litros por Minuto (L/min).
 */
float obterVazaoLMin();

/**
 * Retorna o volume de água total medido desde a última reinicialização ou zeragem.
 *
 * @return Volume total acumulado em Litros (L).
 */
float obterVolumeTotalL();

/**
 * Zera o contador de volume acumulado. Útil para medir o consumo de uma rega individual.
 */
void zerarVolumeTotal();

/**
 * Força a gravação imediata do volume acumulado de água no LittleFS,
 * caso o volume atual seja diferente do último volume salvo na Flash.
 * Usada antes de reinicializações preventivas do sistema pela autovigilância.
 */
void forcarSalvamentoVolume();

