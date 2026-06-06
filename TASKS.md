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
- [ ] Criar `include/pinosHardware.h` (constantes GPIO e defaults)
- [ ] Criar `include/estruturas.h` (structs de configuração)
- [ ] Atualizar `include/README.md`
- [ ] Commit da Etapa 2

## Etapa 3 — Persistência LittleFS (Leitura/Escrita JSON)
- [ ] Criar `include/persistencia.h`
- [ ] Criar `src/persistencia.cpp`
- [ ] Atualizar READMEs de `/src` e `/include`
- [ ] Commit da Etapa 3

## Etapa 4 — Gerenciamento de Rede Wi-Fi (AP/STA)
- [ ] Criar `include/redeWifi.h`
- [ ] Criar `src/redeWifi.cpp`
- [ ] Atualizar READMEs de `/src` e `/include`
- [ ] Commit da Etapa 4

## Etapa 5 — Sincronização NTP
- [ ] Criar `include/relogioNtp.h`
- [ ] Criar `src/relogioNtp.cpp`
- [ ] Atualizar READMEs de `/src` e `/include`
- [ ] Commit da Etapa 5

## Etapa 6 — Motor de Agendamento (Scheduler)
- [ ] Criar `include/agendador.h`
- [ ] Criar `src/agendador.cpp`
- [ ] Atualizar READMEs de `/src` e `/include`
- [ ] Commit da Etapa 6

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
