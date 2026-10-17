#pragma once

/**
 * persistencia.h — Interface do Módulo de Persistência LittleFS
 *
 * Responsável por montar o sistema de arquivos e gerenciar a leitura/escrita
 * dos arquivos de configuração JSON (/network.json e /irrigation.json).
 *
 * Este módulo implementa as decisões do ADR-003 (fonte única de verdade)
 * e ADR-004 (esquemas de persistência). Em caso de arquivo ausente ou
 * corrompido, aplica valores padrão seguros (RF5.1, RF5.2).
 *
 * Requisitos atendidos: ADR-003, ADR-004, ADR-008, RF5.1, RF5.2, CA-04.
 */

#include "estruturas.h"

/**
 * Monta o sistema de arquivos LittleFS.
 *
 * Deve ser chamada no setup() após a inicialização dos relés (ADR-007).
 * Em caso de falha, registra erro via Serial (ADR-008) e retorna false,
 * permitindo que o sistema entre em modo de segurança.
 *
 * @return true se o LittleFS foi montado com sucesso, false caso contrário.
 */
bool inicializarLittleFS();

/**
 * Carrega as configurações de rede a partir do /network.json.
 *
 * Se o arquivo não existir ou estiver corrompido, preenche a struct
 * com os valores padrão do AP (SSID_AP_PADRAO, SENHA_AP_PADRAO) e
 * NTP (SERVIDOR_NTP_PADRAO, etc.), garantindo que o sistema entre
 * em modo AP funcional (RF5.1).
 *
 * @param configRede Referência para a struct que receberá os dados.
 * @return true se o arquivo foi lido com sucesso, false se defaults foram aplicados.
 */
bool carregarConfiguracaoRede(ConfiguracaoRede& configRede);

/**
 * Salva as configurações de rede no /network.json.
 *
 * Serializa a struct para JSON e persiste no LittleFS.
 * Em caso de falha na escrita, registra erro via Serial (ADR-008).
 *
 * @param configRede Referência constante com os dados a salvar.
 * @return true se a escrita foi bem-sucedida.
 */
bool salvarConfiguracaoRede(const ConfiguracaoRede& configRede);

/**
 * Carrega as configurações de irrigação a partir do /irrigation.json.
 *
 * Se o arquivo não existir ou estiver corrompido, preenche a struct
 * com todas as sessões desabilitadas e simultaneidade desativada,
 * garantindo que nenhum relé seja acionado automaticamente (RF5.2).
 *
 * @param configIrrigacao Referência para a struct que receberá os dados.
 * @return true se o arquivo foi lido com sucesso, false se defaults foram aplicados.
 */
bool carregarConfiguracaoIrrigacao(ConfiguracaoIrrigacao& configIrrigacao);

/**
 * Salva as configurações de irrigação no /irrigation.json.
 *
 * Serializa a struct para JSON e persiste no LittleFS.
 * Em caso de falha na escrita, registra erro via Serial (ADR-008).
 *
 * @param configIrrigacao Referência constante com os dados a salvar.
 * @return true se a escrita foi bem-sucedida.
 */
bool salvarConfiguracaoIrrigacao(const ConfiguracaoIrrigacao& configIrrigacao);

