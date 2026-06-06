# Diretório `/include` — Headers e Páginas Web

## Objetivo

Este diretório contém os arquivos de cabeçalho (`.h`) com declarações de funções, estruturas de dados e constantes de hardware do firmware. Também armazena os arquivos HTML da interface web, que são automaticamente convertidos para headers C++ (`PROGMEM`) pelo script `html2c.py` durante o build.

## Conteúdo

| Arquivo | Responsabilidade |
|---|---|
| [pinosHardware.h](pinosHardware.h) | Constantes de GPIOs, limites e valores padrão do hardware. |
| [estruturas.h](estruturas.h) | Structs que mapeiam os esquemas JSON do LittleFS (ADR-004). |
| [persistencia.h](persistencia.h) | Interface do módulo de persistência LittleFS (leitura/escrita JSON). |
| [redeWifi.h](redeWifi.h) | Interface do módulo de gerenciamento Wi-Fi (AP/STA). |
| [relogioNtp.h](relogioNtp.h) | Interface do módulo de sincronização NTP. |
| [agendador.h](agendador.h) | Interface do motor de agendamento (Scheduler). |
| [servidorWeb.h](servidorWeb.h) | Interface do servidor web embarcado (HTTP). |
| [index.html](index.html) | Dashboard principal (convertido para PROGMEM no build). |
| [horarios.html](horarios.html) | Página de programação de horários (convertida para PROGMEM). |
| [config.html](config.html) | Página de configuração de rede e NTP (convertida para PROGMEM). |
| `README` | Guia original do PlatformIO (mantido para referência). |

> **Nota:** Headers e HTMLs serão adicionados conforme as etapas de implementação avançam.

## Dependências / Relações

Os headers deste diretório são consumidos pelos arquivos `.cpp` em `/src`. O fluxo de conversão das páginas web segue o pipeline:

```mermaid
flowchart LR
    HTML["*.html (include/)"] -->|html2c.py| H["*.h (PROGMEM)"]
    H --> SRC["servidorWeb.cpp (src/)"]
    SRC --> ESP["ESP8266 Flash"]
```

Este pipeline é executado automaticamente pelo PlatformIO antes da compilação, graças ao `extra_scripts = html2c.py` no `platformio.ini`.
