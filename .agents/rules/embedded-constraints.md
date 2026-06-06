---
trigger: always_on
---

# Diretrizes para Sistemas Embarcados e Restrições de Hardware

## O Contexto do Microcontrolador
Todo código gerado deve assumir um ambiente de severa restrição de recursos (ex: ESP8266 ou ESP32). A CPU possui clock baixo, a memória RAM (Heap) é extremamente limitada e o sistema operacional subjacente (RTOS ou Non-OS SDK) possui um *Watchdog Timer* (WDT) agressivo.

## 1. Gerenciamento de Memória (O Fim da Fragmentação)
- **Evite a classe `String`:** O uso excessivo da classe `String` nativa do Arduino para concatenação dinâmica fragmenta rapidamente o *Heap*, causando travamentos inexplicáveis após dias de operação.
- **Alternativas Seguras:** Priorize o uso de `snprintf` com buffers estáticos de `char[]`. Se for estritamente necessário usar `String`, utilize `String.reserve()` antes de concatenar.
- **Armazenamento Estático:** Todo conteúdo imutável (páginas HTML, CSS, JavaScript, mensagens de log constantes) **deve** ser armazenado na memória Flash utilizando a macro `PROGMEM`.

## 2. Execução Não-Bloqueante (Watchdog Friendly)
- **Proibição de `delay()`:** O uso de `delay()` para controle de fluxo lógico é estritamente proibido.
- **Máquinas de Estado:** Todo agendamento, espera ou temporização deve ser implementado de forma assíncrona utilizando `millis()` e controle de estado (*State Machines*).
- **Yield / Alimentação do WDT:** Loops pesados (como processamento de grandes arrays JSON ou varreduras no LittleFS) devem incluir chamadas a `yield()` para permitir que as pilhas de rede (Wi-Fi) operem sem acionar o Watchdog.

## 3. Desenvolvimento Web Restrito (Frontend Embarcado)
- **Zero Dependências Externas:** A interface web do equipamento não pode depender de CDNs para carregar bibliotecas (Bootstrap, React, Google Fonts). O sistema deve ser 100% funcional em uma rede local sem acesso à internet.
- **Minimalismo e Custo de Rede:** Páginas HTML, CSS e JavaScript devem ser enxutas. Prefira Vanilla JS com a Fetch API para atualizar dados dinâmicos da página de forma assíncrona, evitando recarregamentos completos da página (Full Page Reloads) que estressam o servidor web do microcontrolador.
- **APIs RESTful Leves:** O microcontrolador deve expor endpoints simples (ex: `/api/status`, `/api/config`) que retornem JSON minimalista, deixando o trabalho de renderização de interface para o navegador do cliente.

## 4. Segurança Elétrica e Física
- **Estados Seguros no Boot:** O hardware atua no mundo físico. A inicialização de pinos (GPIOs) deve garantir um estado de segurança imediato (ex: relés desligados) logo nas primeiras linhas do `setup()`, antes mesmo de iniciar o sistema de arquivos ou o rádio Wi-Fi.
- **Fail-Safe:** Caso os arquivos de configuração do usuário (`.json`) estejam ausentes ou corrompidos, o agente deve garantir que o sistema não assuma comportamentos aleatórios, entrando em um modo de segurança previsível.