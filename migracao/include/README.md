# Diretório `/migracao/include` — Headers da Versão ESP-IDF

## Objetivo

Este diretório armazena os arquivos de cabeçalho (`.h`) da versão em **ESP-IDF**, contendo as declarações de constantes de hardware, protótipos de tarefas do FreeRTOS e estruturas C/C++.

## Conteúdo

| Arquivo | Responsabilidade |
|---|---|
| [pinosHardware.h](pinosHardware.h) | Definição de GPIOs e constantes no padrão `driver/gpio.h`. |
| [estruturas.h](estruturas.h) | Estruturas de agendamento e rede C. |
| [agendador.h](agendador.h) | Protótipos da tarefa de agendamento e auto-verificação anti-ruído. |
| [autovigilancia.h](autovigilancia.h) | Protótipos da tarefa de monitoramento de heap e reset. |
| [redeWifi.h](redeWifi.h) | Interface da pilha `esp_wifi`. |
| [relogioNtp.h](relogioNtp.h) | Interface `esp_sntp`. |
| [sensorFluxo.h](sensorFluxo.h) | Interrupção de GPIO com `gpio_isr_handler_add`. |
| [clienteMqtt.h](clienteMqtt.h) | Interface do `esp_mqtt_client`. |
| [servidorWeb.h](servidorWeb.h) | Interface do `esp_http_server`. |

## Relações de Dependências

```mermaid
flowchart LR
    H["Headers (.h) em /include"] -->|Incluídos por| C["Arquivos C (.c) em /src"]
    C -->|Compilados por| CMAKE["CMake Component 'main'"]
```
