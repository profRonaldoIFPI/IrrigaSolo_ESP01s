#pragma once

/**
 * servidorWeb.h — Interface do Módulo de Servidor Web Embarcado
 *
 * Configura o ESP8266WebServer para servir páginas HTML (armazenadas
 * em PROGMEM via html2c.py) e expor endpoints HTTP para comunicação
 * com o frontend via Fetch API (Vanilla JS).
 *
 * As páginas HTML são incluídas como arrays de caracteres constantes
 * gerados automaticamente pelo script html2c.py durante o build
 * (ADR-002). O frontend utiliza polling AJAX para atualizar dados
 * dinâmicos sem recarregar a página inteira.
 *
 * Requisitos atendidos: RF2.2, RF3.1, RF3.2, RF4.1, RF4.2, CA-03.
 */

#include "estruturas.h"

/**
 * Inicializa o servidor web HTTP na porta 80.
 *
 * Registra todos os handlers de rota (páginas e APIs) e inicia
 * o servidor. Deve ser a última função chamada no setup(), após
 * todos os módulos estarem operacionais.
 *
 * @param configRede       Ponteiro para a struct de rede (leitura pelo API).
 * @param configIrrigacao  Ponteiro para a struct de irrigação (leitura/escrita pelo API).
 */
void inicializarServidor(ConfiguracaoRede* configRede, ConfiguracaoIrrigacao* configIrrigacao);

/**
 * Processa requisições HTTP pendentes.
 *
 * Deve ser chamada no loop() principal a cada iteração.
 * Não-bloqueante: retorna imediatamente se não houver requisições.
 */
void processarRequisicoes();
