#pragma once

/**
 * autovigilancia.h — Interface do Módulo de Autovigilância (Watchdog)
 *
 * Este módulo provê proteção de software e resiliência ao sistema de forma didática,
 * ajudando a prevenir travamentos por memory leak (vazamento de memória) ou falhas
 * na pilha de conexões físicas de rede.
 *
 * Funcionalidades principais:
 *   - Monitoramento contínuo da memória RAM livre (Heap) e reboot em estado crítico
 *   - Monitoramento contínuo do rádio Wi-Fi em modo STA com reset se travar desconectado por > 1h
 *   - Reinicialização preventiva programada a cada 24 horas (uptime de segurança)
 *
 * O reinício só é disparado se a válvula solenoide (relé) estiver desligada,
 * impedindo a interrupção abrupta de irrigações em andamento.
 *
 * Idioma: Português
 * Convenção de nomenclatura: camelCase para funções e PascalCase para tipos.
 */

#include <Arduino.h>

/**
 * Retorna uma string descritiva contendo o motivo do último reset de hardware/software.
 *
 * @return Ponteiro para a string legível com o motivo do reset.
 */
const char* obterMotivoReset();

/**
 * Inicializa as variáveis e timestamps do monitoramento de saúde do sistema.
 * Deve ser chamado ao final da função setup() no arquivo main.cpp.
 */
void inicializarAutovigilancia();

/**
 * Executa as validações e verificações periódicas no Super Loop do microcontrolador.
 * Deve ser chamado a cada iteração da função loop() no arquivo main.cpp de forma não-bloqueante.
 */
void processarAutovigilancia();
