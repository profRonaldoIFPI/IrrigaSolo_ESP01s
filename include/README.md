# Diretório `/include` — Headers e Páginas Web

## Objetivo

Este diretório contém os arquivos de cabeçalho (`.h`) com declarações de funções, estruturas de dados e constantes de hardware do firmware. Também armazena os arquivos HTML da interface web, que são automaticamente convertidos para headers C++ (`PROGMEM`) pelo script `html2c.py` durante o build.

## Conteúdo

| Arquivo | Responsabilidade |
|---|---|
| [pinosHardware.h](pinosHardware.h) | Constantes de GPIOs, limites e valores padrão do hardware. |
| [estruturas.h](estruturas.h) | Structs que mapeiam os esquemas JSON do LittleFS (ADR-004). |
| [persistencia.h](persistencia.h) | Interface do módulo de persistência LittleFS (configurações e volume). |
| [redeWifi.h](redeWifi.h) | Interface do módulo de gerenciamento Wi-Fi (AP/STA). |
| [relogioNtp.h](relogioNtp.h) | Interface do módulo de sincronização NTP. |
| [agendador.h](agendador.h) | Interface do motor de agendamento (Scheduler) e proteção anti-ruído elétrico. |
| [servidorWeb.h](servidorWeb.h) | Interface do servidor web embarcado (HTTP). |
| [sensorFluxo.h](sensorFluxo.h) | Interface de cálculo de vazão e volume com suporte a persistência sob demanda. |
| [clienteMqtt.h](clienteMqtt.h) | Interface de publicação e callback do cliente MQTT (telemetria, reboot e agendamentos). |
| [autovigilancia.h](autovigilancia.h) | Interface do watchdog de heap, rádio Wi-Fi, motivo de reset e reset diário de 24h. |
| [index.html](index.html) | Dashboard principal (convertido para PROGMEM no build). |
| [horarios.html](horarios.html) | Página de programação de horários (convertida para PROGMEM). |
| [config.html](config.html) | Página de configuração de rede e NTP (convertida para PROGMEM). |
| [templates/](templates/) | Parciais HTML compartilhados (`head.html`, `footer.html`). |


> **Nota:** Headers e HTMLs são atualizados e convertidos automaticamente.

## Dependências / Relações

Os headers deste diretório são consumidos pelos arquivos `.cpp` em `/src`. O fluxo de conversão das páginas web com suporte a blocos parciais compartilhados (partials) segue o pipeline:

```mermaid
flowchart TD
    Partials["Partials (templates/head.html, footer.html)"] -->|html2c.py| Merge["Mesclagem em placeholders"]
    HTML["Páginas HTML (index.html, etc)"] -->|html2c.py| Merge
    Merge --> H["Geração de *.h (PROGMEM)"]
    H --> SRC["servidorWeb.cpp (src/)"]
    SRC --> ESP["ESP8266 Flash"]
```

Este pipeline é executado automaticamente pelo PlatformIO antes da compilação, graças ao `extra_scripts = html2c.py` no `platformio.ini`.
