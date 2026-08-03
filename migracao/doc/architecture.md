# Arquitetura do Sistema IrrigaSolo — Versão ESP8266 RTOS SDK / ESP-IDF

Este documento registra as decisões técnicas e arquiteturais (ADR) adotadas para a implementação nativa baseada no **ESP8266 RTOS SDK** (FreeRTOS) e **ESP-IDF** no microcontrolador NodeMCU ESP-12E (v0.9).

---

# ADR-IDF-001 - Arquitetura Multitask Baseada em FreeRTOS (`xTaskCreate`)

### Decisão
Em vez de um Super Loop único com `millis()`, o sistema é estruturado em tarefas paralelas do FreeRTOS gerenciadas pelo escalonador:
1. `tarefaAgendadorIdf`: Executada a cada 1s (`vTaskDelayUntil`) para avaliar agendamentos e executar proteção anti-ruído no pino do relé.
2. `tarefaAutovigilanciaIdf`: Monitora periodicamente a memória Heap (`esp_get_free_heap_size`) e estatísticas do sistema a cada 10s.
3. `tarefaWifi`: Gerenciada pelo Event Loop nativo (`esp_event_handler`).
4. `tarefaMqtt`: Gerenciada pelo componente nativo `esp_mqtt_client`.

### Motivação
Garantir determinismo de tempo real e impedir que travamentos de rede afetem o agendamento ou a proteção física da válvula.

---

# ADR-IDF-002 - Ponto de Entrada `app_main(void)` e Memória NVS

### Decisão
O firmware utiliza a função padrão `void app_main(void)` como ponto de inicialização, responsável por:
1. Desativar imediatamente o relé (`gpio_set_level(PINO_RELE_CANAL_1, 0)`).
2. Inicializar a memória NVS (`nvs_flash_init()`).
3. Instanciar as tarefas FreeRTOS via `xTaskCreate()`.

---

# ADR-IDF-003 - Abstração Nativa de GPIO (`driver/gpio.h`)

### Decisão
As chamadas `digitalWrite()` e `digitalRead()` são substituídas diretamente pelas funções nativas do SDK:
- `gpio_config()` para inicialização dos pinos.
- `gpio_set_level(PINO_RELE_CANAL_1, 0)` para desativar o relé.
- `gpio_get_level(PINO_RELE_CANAL_1)` para leitura rápida da porta física contra ruído elétrico.

---

# ADR-IDF-004 - Persistência de Dados em Memória NVS e SPIFFS

### Decisão
As configurações de rede e agendamento são gravadas em partições NVS (Non-Volatile Storage) / SPIFFS do ESP8266 através das APIs nativas do SDK.

---

# ADR-IDF-005 - Sincronização de Tempo com SNTP Nativo (`esp_sntp`)

### Decisão
A sincronização do relógio é feita através da API nativa `esp_sntp`, operando em segundo plano sem bloquear as demais tarefas.

---

# ADR-IDF-006 - Interrupção por Hardware para Sensor de Fluxo (`gpio_isr_handler_add`)

### Decisão
A leitura de pulsos do sensor de fluxo YF-S201B é realizada registrando uma rotina ISR alocada em IRAM via `gpio_isr_handler_add()`, garantindo contagem precisa mesmo sob alta frequência de pulsos.

---

# ADR-IDF-007 - Cliente MQTT Nativo (`esp_mqtt_client`)

### Decisão
Comunicação MQTT implementada com o componente oficial `esp_mqtt_client`, suportando publicação de telemetria em JSON, subscrição dinâmica e tópicos dedicados de reboot e agendamentos.

---

# ADR-IDF-008 - Proteção Térmica da Válvula Solenoide (Fracionamento e Limite Manual)

### Decisão
1. **Sessões Agendadas Longas (> 10 min):** O firmware fraciona a irrigação em ciclos de 5 minutos ligado e 10 minutos desligado, expandindo a janela total para garantir o tempo útil real.
2. **Controle Manual:** Qualquer acionamento manual é limitado a no máximo 10 minutos contínuos.

---

# ADR-IDF-009 - Autovigilância Anti-Ruído e Diagnóstico de Boot (`esp_reset_reason`)

### Decisão
1. **Auto-verificação Anti-Ruído:** A `tarefaAgendadorIdf` inspeciona o pino físico `GPIO_NUM_5` a cada segundo. Se for encontrado em `HIGH` sem autorização, desliga-o imediatamente (`LOW`) e incrementa o contador de disparos espúrios.
2. **Diagnóstico de Boot:** Na inicialização, a razão de reset é capturada via `esp_reset_reason()` e publicada no tópico `<mqttPubTopic>/reboot`.

---

# ADR-IDF-010 - Auto-Recuperação Periódica de Wi-Fi STA em Modo Dual AP/STA

### Decisão
Em caso de perda de sinal do roteador, o sistema entra em modo dual `WIFI_AP_STA` mantendo o AP `192.168.4.1` ativo, e executa tentativas periódicas não-bloqueantes de reconexão a cada 20 segundos. Assim que o roteador retorna, re-estabelece a conexão STA automaticamente.
