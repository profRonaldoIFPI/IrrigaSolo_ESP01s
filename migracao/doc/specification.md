# Especificação do Sistema IrrigaSolo — Versão ESP8266 RTOS SDK / ESP-IDF

Projeto de sistema de irrigação automatizado baseado em agendamento de horários, controle via interface web local e monitoramento/controle remoto via protocolo MQTT na internet, utilizando o microcontrolador **NodeMCU ESP-12E (v0.9)** com o framework **ESP8266 RTOS SDK** (FreeRTOS) e **ESP-IDF**.

---

## 1. Componentes de Hardware

* **Microcontrolador**: NodeMCU ESP-12E (v0.9) (ESP8266 80MHz, 80KB RAM).
* **Módulo Relé**: 1 canal com isolamento por optoacoplador — **GPIO 5 (D1)** (`GPIO_NUM_5`).
* **Válvula Solenoide**: 1 Válvula solenoide simples (tipo máquina de lavar roupas - 220V).
* **Sensor de Fluxo**: Sensor Hall YF-S201B (3 fios: VCC, GND, Sinal) — **GPIO 14 (D5)** (`GPIO_NUM_14`).
* **Fonte de Alimentação**: 5V regulada.

---

## 2. Requisitos Funcionais

### RF1 - Agendamento e Acionamento (FreeRTOS Task)

O sistema deve permitir o agendamento de sessões de irrigação para o canal único de irrigação gerenciado pela `tarefaAgendadorIdf`.

* **RF1.1:** O canal suporta até 6 sessões de irrigação por dia (início em HH:MM e duração em minutos).
  * Se a duração de uma sessão for menor ou igual a 10 minutos, o relé operará de forma contínua.
  * Se a duração for superior a 10 minutos, o acionamento será fracionado para resfriamento térmico da solenoide em ciclos de 5 minutos ligado e 10 minutos desligado (período total de 15 minutos do ciclo).
  * Para garantir que a duração configurada reflita o tempo real de irrigação (água ligada), a janela total de tempo decorrido da sessão será expandida (esticada) proporcionalmente aos períodos de descanso.
* **RF1.2:** A base de tempo deve ser sincronizada via SNTP nativo do SDK (`esp_sntp`), com consultas periódicas. Em caso de falha de rede, o sistema mantém o relógio local.
* **RF1.3:** Recuperação de Falha de Energia: O sistema não deve acumular ou repor regas perdidas durante quedas de energia. Ao reiniciar, aguarda-se o próximo horário programado validado pelo relógio atual.
* **RF1.4:** Proteção no Acionamento Manual: Qualquer acionamento manual de válvula (via interface web ou comando MQTT) é limitado a no máximo 10 minutos contínuos, desligando-se automaticamente após esse período.

### RF2 - Gerenciamento de Rede e Configuração Inicial (`esp_wifi`)

O dispositivo deve gerenciar sua conexão Wi-Fi de forma autônoma utilizando a pilha nativa `esp_wifi` e manipuladores de evento `esp_event`.

* **RF2.1:** Na ausência de rede conhecida, opera em modo Access Point (SSID padrão: `IrrigaSolo`, Senha: `admin123`) e exibe um Portal Cativo para configuração.
* **RF2.2:** A interface inicial deve coletar: SSID e Senha da rede local, URL do servidor NTP (padrão `pool.ntp.br`), Fuso Horário, Intervalo de Sincronização e os parâmetros de conexão MQTT (Broker, Porta, Usuário, Senha, Client ID, Tópico de Publicação, Tópico de Assinatura e Intervalo de Publicação).
* **RF2.3:** Em caso de perda de conexão STA (rede local indisponível/reboot do roteador), o sistema aciona o fallback para o modo dual AP/STA (`WIFI_AP_STA`), mantendo tentativas periódicas não-bloqueantes de reconexão ao SSID configurado a cada 20 segundos. Assim que a rede for restaurada, o dispositivo retorna automaticamente ao modo STA pleno.

### RF3 - Medição de Fluxo de Água (ISR `driver/gpio.h`)

O sistema deve integrar o sensor de fluxo YF-S201B via interrupção de hardware por borda de descida (`gpio_isr_handler_add`).

* **RF3.1:** O firmware deve calcular a vazão instantânea de água em litros por minuto (L/min).
* **RF3.2:** O sistema deve acumular o volume total de água consumido (em litros) durante o acionamento da válvula.
* **RF3.3:** O volume total acumulado pode ser zerado via interface web local ou comando MQTT.

### RF4 - Interface Web (Painel de Controle `esp_http_server`)

A interface deve ser servida via componente nativo `esp_http_server`.

* **RF4.1:** **Painel Principal:** Exibe status do Wi-Fi, conexão MQTT, hora atual do sistema, estado da válvula e medições em tempo real do sensor de fluxo (vazão e volume total). Permite acionamento manual (Ligar/Desligar).
* **RF4.2:** **Painel de Horários:** Interface para cadastrar, editar, habilitar ou desabilitar as 6 sessões do canal de irrigação.
* **RF4.3:** **Painel de Configuração:** Interface para alterar credenciais de rede, NTP e as configurações do Broker MQTT.

### RF5 - Comunicação MQTT (`esp_mqtt_client`)

O sistema deve utilizar o componente nativo `esp_mqtt` do ESP-IDF.

* **RF5.1:** **Publicação de Telemetria:** O dispositivo deve publicar periodicamente o estado no tópico configurado no formato JSON:
  `{"valvula": "LIGADA"|"DESLIGADA", "vazao": 0.0, "volume": 0.0, "motivo_reset": "...", "disparos_espurios": 0}`
* **RF5.2:** **Recepção de Comandos e Agendamentos via MQTT:** O dispositivo aceita comandos (`LIGAR`, `DESLIGAR`, `ZERAR`, `AGENDAMENTOS`, `REBOOT`) e cadastro/atualização de agendamentos via payload JSON.
* **RF5.3:** **Tópicos Dedicados de Reboot, Agendamentos e Inscrição de Agendar**:
  * Tópico de Reboot (`<mqttPubTopic>/reboot`): Publica payload JSON com motivo de reinício (`esp_reset_reason`), uptime e disparos espúrios.
  * Tópico de Agendamentos (`<mqttPubTopic>/agendamentos`): Publica payload JSON com as 6 sessões de irrigação.
  * Tópico de Inscrição de Agendamento (`<mqttSubTopic>/agendar`): Tópico dedicado para cadastrar agendamentos.

### RF6 - Resiliência e Tolerância a Falhas

* **RF6.1:** Corrupção NVS: Reverte para padrões seguros em modo AP.
* **RF6.2:** Proteção Anti-Ruído e Auto-Verificação do Pino da Válvula (`verificarEProtegerEstadoReleFisicoIdf`): A `tarefaAgendadorIdf` inspeciona continuamente a porta física do relé via `gpio_get_level(PINO_RELE_CANAL_1)`. Caso detecte estado `HIGH` não autorizado, desliga o relé instantaneamente (`gpio_set_level(..., 0)`), incrementa o contador de disparos espúrios e registra o alerta.

---

## 3. Critérios de Aceitação (CA)

* **CA-01:** O relé inicia estritamente em `LOW` (desligado) no primeiro milissegundo do `app_main()`.
* **CA-02:** O agendamento executa em tarefa dedicada do FreeRTOS sem travar a CPU.
* **CA-03:** Interrupção de hardware ISR mede vazão e volume sem perda de pulsos.
* **CA-04:** A reconexão Wi-Fi e MQTT ocorre de forma não-bloqueante via tarefas FreeRTOS.
* **CA-05:** As configurações são salvas com segurança na memória não-volátil NVS.
* **CA-06:** Regas agendadas longas (> 10 min) executam obrigatoriamente de forma fracionada (5 min ligado, 10 min desligado), com a janela de execução expandida para garantir o tempo útil real.
* **CA-07:** O sistema detecta acionamentos espúrios provocados por ruído elétrico e desliga a válvula imediatamente via `gpio_set_level`.
* **CA-08:** O motivo de reinício capturado via `esp_reset_reason()` é publicado no tópico `<mqttPubTopic>/reboot`.
