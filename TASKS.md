# Tarefas do Projeto IrrigaSolo

Quadro de planejamento central para acompanhar a evolução incremental do firmware.
Cada etapa concluída recebe um commit isolado conforme o padrão do projeto.

---

## Etapa 1 — Infraestrutura do Repositório
- [x] Inicializar repositório Git
- [x] Criar `TASKS.md`
- [x] Criar `README.md` na raiz do projeto
- [x] Criar `README.md` em `/src`
- [x] Criar `README.md` em `/include`
- [x] Criar `README.md` em `/lib`
- [x] Criar `README.md` em `/doc`
- [x] Criar `README.md` em `/test`
- [x] Commit da Etapa 1

## Etapa 2 — Definições de Hardware e Estruturas de Dados
- [x] Criar `include/pinosHardware.h` (constantes GPIO e defaults)
- [x] Criar `include/estruturas.h` (structs de configuração)
- [x] Atualizar `include/README.md`
- [x] Commit da Etapa 2

## Etapa 3 — Persistência LittleFS (Leitura/Escrita JSON)
- [x] Criar `include/persistencia.h`
- [x] Criar `src/persistencia.cpp`
- [x] Atualizar READMEs de `/src` e `/include`
- [x] Commit da Etapa 3

## Etapa 4 — Gerenciamento de Rede Wi-Fi (AP/STA)
- [x] Criar `include/redeWifi.h`
- [x] Criar `src/redeWifi.cpp`
- [x] Atualizar READMEs de `/src` e `/include`
- [x] Commit da Etapa 4

## Etapa 5 — Sincronização NTP
- [x] Criar `include/relogioNtp.h`
- [x] Criar `src/relogioNtp.cpp`
- [x] Atualizar READMEs de `/src` e `/include`
- [x] Commit da Etapa 5

## Etapa 6 — Motor de Agendamento (Scheduler)
- [x] Criar `include/agendador.h`
- [x] Criar `src/agendador.cpp`
- [x] Atualizar READMEs de `/src` e `/include`
- [x] Commit da Etapa 6

## Etapa 7 — Servidor Web e Interface HTML
- [x] Criar `include/servidorWeb.h`
- [x] Criar `src/servidorWeb.cpp`
- [x] Criar `include/index.html` (Dashboard)
- [x] Criar `include/horarios.html` (Agendamentos)
- [x] Criar `include/config.html` (Rede & NTP)
- [x] Atualizar READMEs de `/src` e `/include`
- [x] Commit da Etapa 7

## Etapa 8 — Integração Final no `main.cpp`
- [x] Implementar `setup()` com sequência ADR-007
- [x] Implementar `loop()` com super loop não-bloqueante
- [x] Atualizar `platformio.ini` (filesystem, monitor_speed)
- [x] Atualizar READMEs finais
- [x] Commit da Etapa 8

## Etapa Extra — Configuração de Permissões de Gravação (Linux)
- [x] Adicionar seção de solução de erro de porta serial no `Anotacoes.md`
- [x] Executar comando no terminal para adicionar usuário ao grupo `dialout`

## Etapa 9 — Transição para Canal Único, Sensor de Fluxo e MQTT (Local & Configurável)
- [x] Atualizar especificações e decisões de arquitetura em `/doc`
- [x] Adicionar campo `mqttInterval` em `estruturas.h`
- [x] Atualizar parsing JSON em `persistencia.cpp`
- [x] Substituir o polling estático pelo configurável no `main.cpp`
- [x] Adicionar campo no formulário HTML de `config.html`
- [x] Ajustar endpoints da API de rede em `servidorWeb.cpp`
- [x] Implementar a função `obterDuracaoEsticada` em `agendador.cpp`
- [x] Atualizar a lógica da janela de execução de sessões em `avaliarAgendamentos`
- [x] Implementar o temporizador de segurança para desligamento automático do controle manual (10 minutos)
- [x] Atualizar o payload JSON de telemetria MQTT com última e próxima irrigação em `clienteMqtt.cpp`
- [x] Compilar e validar a integridade estrutural do firmware

## Etapa 10 — Persistência do Volume de Água (LittleFS)
- [x] Declarar `carregarVolumeAcumulado` e `salvarVolumeAcumulado` em `persistencia.h`
- [x] Implementar leitura e escrita de `/volume.json` em `persistencia.cpp`
- [x] Atualizar `sensorFluxo.h` e `sensorFluxo.cpp` para carregar e salvar de forma inteligente
- [x] Validar compilação e comitar da Etapa 10

## Etapa 11 — Telemetria MQTT Expandida (JSON com Uptime, Heap, IP, RSSI, IDs)
- [x] Ampliar buffer de telemetria MQTT em `clienteMqtt.cpp`
- [x] Adicionar dados adicionais de diagnóstico no payload JSON de telemetria
- [x] Validar e comitar a Etapa 11

## Etapa 12 — Mecanismo de Autovigilância (Watchdog e Reset Programado)
- [x] Criar arquivo de cabeçalho `include/autovigilancia.h`
- [x] Criar arquivo de implementação `src/autovigilancia.cpp` com monitor de heap, rede e reset diário
- [x] Integrar inicialização e processamento em `src/main.cpp`
- [x] Validar e comitar a Etapa 12

## Etapa 13 — Atualização de Documentação dos Diretórios
- [x] Atualizar `src/README.md`
- [x] Atualizar `include/README.md`
- [x] Validar e comitar a Etapa 13

## Etapa 14 — Simplificação da Persistência do Volume de Água
- [x] Remover lógica de salvamento a cada 5L e de salvamento sob demanda no reinício em `sensorFluxo.h` e `sensorFluxo.cpp`
- [x] Remover chamadas de `forcarSalvamentoVolume` em `autovigilancia.cpp`
- [x] Validar compilação e comitar a Etapa 14

## Etapa 15 — Registro de Horário da Última Irrigação Manual e Agendada
- [x] Centralizar a escrita do relé em `setarEstadoReleFisico` em `agendador.cpp` e registrar a hora de início
- [x] Atualizar `obterUltimaIrrigacao` para priorizar o valor registrado em RAM
- [x] Validar compilação e comitar a Etapa 15

## Etapa 16 — Proteção Anti-Ruído, Telemetria de Boot e Tópicos MQTT (Reboot e Agendamentos)
- [x] Atualizar a documentação de especificação e arquitetura em `/doc` com RF5.3, RF6.3 e ADR-012
- [x] Implementar auto-verificação do pino do relé e contador de disparos espúrios em `agendador.h` e `agendador.cpp`
- [x] Registrar motivo de reset de hardware e auto-recuperação Wi-Fi em `autovigilancia.h` e `autovigilancia.cpp`
- [x] Implementar tópicos MQTT dedicados de reboot e agendamentos em `clienteMqtt.h` e `clienteMqtt.cpp`
- [x] Integrar fluxo no Super Loop em `main.cpp`
- [x] Atualizar documentação dos diretórios `src/README.md` e `include/README.md`
## Etapa 17 — Cadastramento de Agendamentos de Irrigação via MQTT (Payload JSON)
- [x] Atualizar requisitos RF5.2, RF5.3 e ADR-012 em `/doc`
- [x] Implementar parser JSON de agendamento em `clienteMqtt.cpp` e inscrição no tópico `<mqttSubTopic>/agendar`
- [x] Persistir os novos agendamentos recebidos via MQTT no LittleFS (`/irrigation.json`)
## Etapa 18 — Exibição da Versão do Firmware no Card OTA (Interface Web)
- [x] Criar constante `VERSAO_FIRMWARE` em `include/pinosHardware.h`
- [x] Retornar `firmwareVersion` nos endpoints de API `/api/status` e `/api/network` em `servidorWeb.cpp`
- [x] Exibir a versão atual no card de Atualização de Firmware (OTA) em `include/templates/config.html`
- [x] Validar compilação com `pio run` e comitar a Etapa 18

## Etapa 19 — Estrutura de Migração Didática para ESP-IDF & FreeRTOS (`/migracao`)
- [x] Criar estrutura de projeto PlatformIO para ESP-IDF (`framework = espidf`, `board = esp32dev`)
- [x] Criar `CMakeLists.txt` raiz e no componente `src`
- [x] Implementar ponto de entrada `app_main()` em `migracao/src/main.c`
- [x] Implementar tarefas do FreeRTOS para agendador, autovigilância e proteção anti-ruído no pino do relé
- [x] Criar documentação didática completa com diagramas Mermaid e comparativo de arquitetura em `migracao/README.md`
- [x] Garantir arquivos `README.md` obrigatórios em todos os subdiretórios (`/migracao/doc`, `/migracao/include`, `/migracao/src`)
## Etapa 20 — Correção de Publicação MQTT (Buffer PubSubClient & Reinicialização Web)
- [x] Expandir o buffer interno da biblioteca `PubSubClient` para 512 bytes via `mqttClient.setBufferSize(512)`
- [x] Preservar o ponteiro de agendamento em `inicializarMqtt()` e passar `ptrConfigIrrigacao` em `servidorWeb.cpp`
## Etapa 21 — Rotina de Auto-Reconexão Wi-Fi STA Não-Bloqueante (Reboot do Roteador)
- [x] Atualizar especificação (RF2.3) e registrar ADR-013 em `/doc`
- [x] Implementar modo dual `WIFI_AP_STA` e ciclo de reconexão a cada 20s em `src/redeWifi.cpp`
- [x] Desativar o AP de fallback e restaurar estado estrito STA quando o roteador volta a operar
- [x] Validar compilação com `pio run` e comitar a Etapa 21

## Etapa 22 — Limpeza do Repositório (Remoção da pasta `/migracao`)
- [x] Confirmar remoção dos arquivos de `/migracao` do Git conforme decisão acordada
- [x] Validar e comitar a Etapa 22

## Etapa 23 — Atualização das Especificações e Arquitetura (`/doc`)
- [x] Atualizar `doc/specification.md` (ESP-01S, módulo relé v4.0 em GPIO 0 Active HIGH, remoção do sensor de fluxo RF3 e CA-03)
- [x] Atualizar `doc/architecture.md` (ADR-001 para ESP-01S 1MB, registrar ADR-014 para adaptação e remoção do sensor, atualizar ADR-007, ADR-010 e ADR-012)
- [x] Atualizar `doc/README.md`
- [x] Validar e comitar a Etapa 23

## Etapa 24 — Configuração da Plataforma PlatformIO para ESP-01S (`platformio.ini`)
- [x] Configurar ambiente `[env:esp01_1m]` com `board = esp01_1m` e mapa LittleFS para 1MB
- [x] Validar e comitar a Etapa 24

## Etapa 25 — Atualização das Constantes de Hardware (`include/pinosHardware.h`)
- [x] Atualizar pinagem para ESP-01S: `PINO_RELE_CANAL_1 = 0` (GPIO 0, Active HIGH) e `PINO_LED_STATUS = 2` (GPIO 2)
- [x] Remover definições do sensor de fluxo (`PINO_SENSOR_FLUXO` e `FATOR_CALIBRACAO_FLUXO`)
- [x] Atualizar `include/README.md`
- [x] Validar e comitar a Etapa 25

## Etapa 26 — Remoção do Módulo do Sensor de Fluxo (`src` e `include`)
- [ ] Remover `include/sensorFluxo.h` e `src/sensorFluxo.cpp`
- [ ] Remover funções de persistência de volume em `include/persistencia.h` e `src/persistencia.cpp`
- [ ] Atualizar `include/README.md` e `src/README.md`
- [ ] Validar e comitar a Etapa 26

## Etapa 27 — Refatoração da Telemetria e Comandos MQTT (`clienteMqtt`)
- [ ] Remover referências e métricas de fluxo/volume do payload JSON de telemetria MQTT em `src/clienteMqtt.cpp`
- [ ] Remover o tratamento do comando remoto `ZERAR`
- [ ] Atualizar `include/clienteMqtt.h`
- [ ] Validar e comitar a Etapa 27

## Etapa 28 — Refatoração do Servidor Web e Templates HTML (`servidorWeb`, `index.html`)
- [ ] Remover `vazao` e `volume` de `/api/status` e remover rota `/api/zerar-volume` em `src/servidorWeb.cpp`
- [ ] Remover cards/displays de vazão e volume em `include/templates/index.html`
- [ ] Executar script gerador `html2c.py` para atualizar `include/index.h`
- [ ] Validar e comitar a Etapa 28

## Etapa 29 — Integração do `main.cpp` e Proteção Anti-Ruído para GPIO 0
- [ ] Remover chamadas de inicialização e processamento do sensor de fluxo em `src/main.cpp`
- [ ] Garantir inicialização segura do relé em `setup()` para GPIO 0 (Active HIGH)
- [ ] Ajustar proteção anti-ruído para monitorar GPIO 0 em `src/agendador.cpp` e `include/agendador.h`
- [ ] Validar e comitar a Etapa 29

## Etapa 30 — Validação da Compilação, Testes e Documentação Geral
- [ ] Compilar projeto completo via `pio run` para ambiente `esp01_1m`
- [ ] Atualizar `README.md` da raiz com as informações do ESP-01S e pinagem
- [ ] Atualizar `Anotacoes.md` com instruções de gravação via adaptador USB-Serial para ESP-01S
- [ ] Atualizar `TASKS.md` finalizando todas as etapas
- [ ] Validar e comitar a Etapa 30










