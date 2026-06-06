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
- [ ] Criar `include/servidorWeb.h`
- [ ] Criar `src/servidorWeb.cpp`
- [ ] Criar `include/index.html` (Dashboard)
- [ ] Criar `include/horarios.html` (Agendamentos)
- [ ] Criar `include/config.html` (Rede & NTP)
- [ ] Atualizar READMEs de `/src` e `/include`
- [ ] Commit da Etapa 7

## Etapa 8 — Integração Final no `main.cpp`
- [ ] Implementar `setup()` com sequência ADR-007
- [ ] Implementar `loop()` com super loop não-bloqueante
- [ ] Atualizar `platformio.ini` (filesystem, monitor_speed)
- [ ] Atualizar READMEs finais
- [ ] Commit da Etapa 8
