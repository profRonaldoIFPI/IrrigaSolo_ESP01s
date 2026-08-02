# Arquitetura do Sistema IrrigaSolo

Este documento descreve as decisões técnicas adotadas para a implementação do firmware. Segue o padrão de Registro de Decisões Arquiteturais (ADR).

---
# ADR-001 - Plataforma e Ecossistema

## Decisão
Utilizar o framework Arduino sobre o microcontrolador ESP8266 (NodeMCU ESP-12E) via **PlatformIO**. 

## Motivação
O ecossistema é maduro, possui excelente suporte da comunidade, compatibilidade nativa com processos OTA e integra-se perfeitamente às ferramentas de desenvolvimento (como a Antigravity IDE) utilizadas no projeto.

---

# ADR-002 - Organização do Código e Interface Web

## Decisão
A interface web não utilizará frameworks frontend externos (sem React, Vue, ou CDNs). O HTML/CSS/JS será escrito de forma pura, convertido para arrays de bytes (`PROGMEM`) usando o script `html2c.py` e servido diretamente da memória Flash do ESP8266.

## Motivação
Eliminar dependência de conectividade externa para carregar assets (garantindo o funcionamento offline em rede local), reduzir o consumo de memória RAM (Heap) e simplificar o processo de build através da estrutura já fornecida pelo PlatformIO.

---

# ADR-003 - Fonte Única de Verdade para Credenciais (Single Source of Truth)

## Decisão
Todas as configurações de rede (tanto AP quanto STA) serão geridas unicamente pelo arquivo de rede no LittleFS. O cache nativo do SDK do ESP8266 para Wi-Fi será explicitamente desabilitado no boot (`WiFi.persistent(false)`).

## Motivação
Evitar estados híbridos onde o LittleFS diz uma coisa e a EEPROM do ESP8266 diz outra. Se o arquivo JSON corromper, o sistema sabe com 100% de certeza que deve voltar ao estado de fábrica (AP Mode), garantindo um comportamento determinístico.

---

# ADR-004 - Persistência de Configuração (Esquemas LittleFS)

## Decisão
As configurações persistentes serão separadas em dois arquivos JSON no LittleFS, carregados durante o boot.

**1. `/network.json`**

```json
{
  "ssidAp": "IrrigaSolo",
  "pwdAp": "admin123",
  "ssidSta": "IoTnet",
  "pwdSta": "Casa20@iot_devices",
  "ntpServer": "pool.ntp.br",
  "timeZone": -3,
  "ntpSyncInterval": 21600,
  "mqttBroker": "192.168.1.49",
  "mqttPort": 1883,
  "mqttUser": "iotdevices",
  "mqttPwd": "iotCasa20",
  "mqttClientId": "irrigasolo_1",
  "mqttPubTopic": "irriga_solo/1/status",
  "mqttSubTopic": "irriga_solo/1/comando",
  "mqttInterval": 10
}
```

(Nota: `ntpSyncInterval` define o tempo em segundos entre sincronizações - ex: 21600s = 6 horas).

**2. `/irrigation.json`**

```json
{
  "channel1": [ { "enabled": true, "start": "06:00", "duration": 10 } ]
}
``` 

### Motivação
Isolamento de domínio. Atualizar horários de irrigação não exige tocar no arquivo de rede, reduzindo o desgaste da memória Flash e o risco de corrupção cruzada.

---

# ADR-005 - Motor de Agendamento (Scheduler Customizado)

### Decisão
Não utilizar bibliotecas de alarmes baseadas em interrupções ou callbacks (como TimeAlarms).
O agendamento será implementado através de um Super Loop (polling) que compara o horário atual gerado pelo NTPClient com os timestamps ou equivalentes numéricos (ex: minutos desde a meia-noite) das sessões ativas no `/irrigation.json`.

### Motivação
Evitar race conditions com processos assíncronos do ESP8266 (WiFi e WebServer).
Maior previsibilidade de execução.
Facilidade para ignorar "regas perdidas" durante quedas de energia (o loop simplesmente avalia: "o horário atual está dentro de uma janela de rega ativa?").

---

# ADR-006 - Integração do Sensor de Fluxo de Água

### Decisão
O monitoramento do fluxo de água utilizará uma rotina de interrupção externa (ISR) associada ao pino do sensor (GPIO14/D5). A ISR incrementará um contador de pulsos global. A cada 1 segundo, o loop principal calculará a vazão instantânea (L/min) baseado na frequência de pulsos e acumulará o volume consumido total em Litros, armazenado em uma variável de ponto flutuante de precisão simples (`float`).

### Motivação
O sensor de fluxo YF-S201B gera pulsos de alta frequência que dependem da vazão de água. O uso de interrupções de hardware garante que nenhum pulso seja perdido durante a execução de outras tarefas (como processamento web ou rede). O cálculo da vazão no loop principal de 1 segundo minimiza o tempo gasto dentro da rotina de interrupção (ISR), mantendo a estabilidade do ESP8266.

---

# ADR-007 - Controle Físico e Sequência de Inicialização

### Decisão
O módulo de relé opera em lógica Active High.
A sequência de inicialização no `setup()` deve ser estritamente:
1. Configurar `pinMode` para o GPIO do relé.
2. Escrever `LOW` imediata e explicitamente no pino.
3. Montar LittleFS.
4. Inicializar o sensor de fluxo (anexar interrupção).
5. (Restante do boot: Rede, NTP, Servidor Web, MQTT).

### Motivação
Garantir que picos de flutuação de pino durante o boot do ESP8266 não acionem a válvula de água indevidamente antes que o sistema tenha consciência de seu próprio estado.

---

# ADR-008 - Telemetria e Logs

### Decisão
Utilizar a porta Serial nativa do ESP8266 (`Serial.print`) formatando a saída com níveis de severidade `[INFO]`, `[WARN]`, `[ERROR]`. Em caso de falha no parser JSON do LittleFS, registrar detalhadamente o nó que falhou.

### Motivação
Permitir depuração em bancada (USB) durante o desenvolvimento e diagnósticos rápidos de falha estrutural de arquivos de configuração.

---

# ADR-009 - Mecanismo de Atualização Remota (OTA)

### Decisão
Implementar a atualização remota (OTA - *Over-The-Air*) baseada em interface web local via biblioteca nativa `ESP8266HTTPUpdateServer` mapeada na rota `/update`.

### Motivação
A escolha de um atualizador web local (pull via interface de usuário) ao invés do protocolo nativo do ArduinoOTA (push via porta local do computador) se deve a:
1. **Acessibilidade Universal:** O usuário/desenvolvedor pode atualizar o dispositivo estando tanto na rede local do roteador quanto conectado diretamente no Access Point nativo do ESP8266, sem precisar de ferramentas de build complexas instaladas.
2. **Segurança de Rede e Firewall:** Evita falhas no processo causadas por firewalls do sistema operacional do host bloqueando portas efêmeras de escuta exigidas pelo ArduinoOTA.
3. **Padrão de Mercado:** Segue o fluxo clássico adotado por roteadores e equipamentos residenciais inteligentes, onde o firmware é atualizado por meio de um arquivo `.bin` na página administrativa do equipamento.

---

# ADR-010 - Protocolo MQTT para Telemetria e Controle Remoto

### Decisão
Utilizar a biblioteca `PubSubClient` para realizar a conexão MQTT com um broker externo. A conexão será não-bloqueante, tentando restabelecer o link periodicamente (a cada 10 segundos) sem o uso de `delay()`. O dispositivo enviará telemetrias periódicas a cada 5 segundos no formato JSON e receberá comandos para ligar/desligar o relé ou zerar o volume acumulado.

### Motivação
Integrar o sistema à internet de forma eficiente e leve. O uso do protocolo MQTT permite baixo consumo de banda e uma arquitetura baseada em eventos (publicador/assinante), ideal para o monitoramento e controle em tempo real via internet a partir de aplicativos ou brokers gratuitos de nuvem.

---

# ADR-011 - Proteção Térmica da Válvula Solenoide (Fracionamento e Limite Manual)

### Decisão
Para proteger fisicamente a válvula solenoide (tipo máquina de lavar roupas) contra superaquecimento:
1. **Sessões Agendadas Longas (> 10 min):** O firmware irá fracionar a irrigação automaticamente em ciclos periódicos de 5 minutos ligado e 10 minutos desligado (período total de 15 minutos do ciclo). A janela total da sessão é expandida (esticada) para garantir que o tempo útil configurado seja integralmente cumprido com a válvula aberta.
2. **Controle Manual:** Qualquer ativação manual do relé (via web ou MQTT) terá um temporizador rígido que desligará o dispositivo automaticamente após 10 minutos contínuos.

### Motivação
Essas válvulas solenoides residenciais não são projetadas para regime de operação contínuo longo e podem queimar por dissipação térmica inadequada. A implementação desse recurso no nível do firmware impede falhas de hardware no mundo físico.

---

# ADR-012 - Autovigilância Anti-Ruído, Diagnóstico de Boot e Tópicos MQTT Específicos

### Decisão
1. **Auto-verificação Anti-Ruído (Hardware Safety Check):** O motor de agendamento inspeciona o estado lógico do pino GPIO 5 (D1). Caso o pino físico seja encontrado em `HIGH` sem autorização explícita (sem agendamento ativo e sem override manual), o firmware desliga o pino instantaneamente (`LOW`), incrementa o contador de disparos espúrios e gera logs de alerta.
2. **Diagnóstico de Boot, Agendamento via JSON e Tópicos MQTT Dedicados:**
   - Na inicialização e reconexão MQTT, o sistema publica a razão de reset de hardware (`ESP.getResetReason()`), tempo online e disparos espúrios no tópico `<mqttPubTopic>/reboot`.
   - O sistema publica as 6 sessões de irrigação ativas no tópico `<mqttPubTopic>/agendamentos` e responde ao comando `AGENDAMENTOS`.
   - O sistema permite o cadastramento de novos agendamentos via payload JSON (ex: `{"sessao":0,"habilitada":true,"inicio":"06:30","duracao":15}`) no tópico de assinatura ou `<mqttSubTopic>/agendar`, persistindo os dados imediatamente no LittleFS.
   - As métricas `motivo_reset` e `disparos_espurios` são integradas no payload JSON de telemetria geral.

### Motivação
Equipamentos eletrodomésticos de alta potência (como compressores de ar-condicionado) induzem ruído eletromagnético e surtos na rede elétrica que podem comutar transistores de relés ou causar flutuações de GPIO. A verificação ativa contínua garante proteção de hardware com tempo de resposta em milissegundos. A telemetria dedicada permite rastreabilidade remota de reinicializações e falhas da rede elétrica.


