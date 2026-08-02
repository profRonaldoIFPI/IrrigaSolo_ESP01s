# Arquitetura ESP-IDF do Sistema IrrigaSolo

Este documento registra as decisões arquiteturais adotadas para a versão industrial baseada em **ESP-IDF** e **FreeRTOS** no microcontrolador ESP32.

---

# ADR-IDF-001 - Divisão em Tarefas FreeRTOS (Task-Based Architecture)

### Decisão
Em vez de um Super Loop único (`loop()`), o sistema é organizado em 4 tarefas principais concorrentes:
1. `tarefaAgendador`: Executa o polling de 1s para verificação de horários e proteção anti-ruído no pino do relé.
2. `tarefaWiFi`: Gerencia os eventos de estado de rede via `esp_event_handler`.
3. `tarefaMqtt`: Gerencia o cliente MQTT nativo `esp_mqtt_client`.
4. `tarefaAutovigilancia`: Monitora heap, contadores de reset e dispara alertas de saúde.

### Motivação
Garantir previsibilidade de tempo real e alta disponibilidade da pilha Wi-Fi sem que bloqueios em rede afetem a proteção de hardware do relé.

---

# ADR-IDF-002 - Ponto de Entrada `app_main(void)`

### Decisão
O firmware utiliza a função padrão `void app_main(void)` como ponto de inicialização do sistema, responsável por:
1. Inicializar a memória NVS (`nvs_flash_init()`).
2. Configurar o driver de GPIO dos relés e sensores.
3. Criar as tarefas do FreeRTOS via `xTaskCreate()`.

---

# ADR-IDF-003 - Abstração do Driver de GPIO (`driver/gpio.h`)

### Decisão
As chamadas `digitalWrite()` e `digitalRead()` são substituídas diretamente pelas funções nativas do SDK:
- `gpio_config()` para inicialização.
- `gpio_set_level(PINO_RELE_CANAL_1, 0)` para desativar a válvula.
- `gpio_get_level(PINO_RELE_CANAL_1)` para leitura rápida da porta física.
