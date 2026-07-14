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
- [x] Atualizar struct `ConfiguracaoRede` em `estruturas.h`
- [x] Modificar padrões e parsing JSON em `persistencia.cpp`
- [x] Atualizar lógica de conexão e publicação em `clienteMqtt.cpp`
- [x] Modificar interface de configuração de rede em `config.html`
- [x] Ajustar endpoints da API de rede em `servidorWeb.cpp`
- [x] Compilar e validar a integridade estrutural do firmware


