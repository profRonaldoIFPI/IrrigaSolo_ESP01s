# Arquitetura do Sistema IrrigaSolo

Este documento descreve as decisões técnicas adotadas para a implementação do firmware. Segue o padrão de Registro de Decisões Arquiteturais (ADR).

---

# ADR-001 - Plataforma e Ecossistema (ESP-01S)

## Decisão
Utilizar o framework Arduino sobre o microcontrolador **ESP8266 ESP-01S** (com 1MB de memória Flash SPI) via **PlatformIO** (ambiente `esp01_1m`), configurando explicitamente o modo de acesso à Flash SPI como **DOUT** (`board_build.flash_mode = dout`).

## Motivação
O ecossistema ESP8266 é maduro, possui excelente suporte da comunidade, compatibilidade nativa com processos OTA e permite a criação de nós de automação ultracompactos de baixo custo. O ESP-01S, acoplado diretamente a um módulo relé dedicado, viabiliza uma solução autônoma com footprint físico mínimo.

A especificação explícita do modo **DOUT** é mandatória porque a grande maioria dos módulos ESP-01S modernos (frequentemente equipados com memórias Flash PUYA ou similares de 1MB) não opera de forma confiável em modo Quad I/O (QIO) durante a leitura do bootloader da ROM. A inicialização em modo QIO padrão resulta em travamento cíclico de reinicialização (*boot loop / crash*), impedindo o microcontrolador de carregar o código de usuário, conectar ao Wi-Fi ou iniciar o Access Point.

---

# ADR-002 - Organização do Código e Interface Web

## Decisão
A interface web não utilizará frameworks frontend externos (sem React, Vue, ou CDNs). O HTML/CSS/JS será escrito de forma pura, convertido para arrays de bytes (`PROGMEM`) usando o script `html2c.py` e servido diretamente da memória Flash do ESP-01S.

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
As configurações persistentes serão separadas em dois arquivos JSON no LittleFS (partição de 64KB no 1MB de flash), carregados durante o boot.

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

# ADR-006 - [REVOGADO] Integração do Sensor de Fluxo de Água

*(Revogado e substituído pelo ADR-014 devido à migração de hardware para o ESP-01S).*

---

# ADR-007 - Controle Físico e Sequência de Inicialização no ESP-01S

### Decisão
O módulo de relé v4.0 para ESP-01S é acionado no pino **GPIO 0** em lógica **Active HIGH** (nível ALTO liga o relé, nível BAIXO desliga o relé).
O LED indicador onboard do ESP-01S reside no **GPIO 2** em lógica **Active LOW** (nível BAIXO acende, nível ALTO apaga).

A sequência de inicialização no `setup()` deve ser estritamente:
1. Configurar `pinMode(PINO_RELE_CANAL_1, OUTPUT)` e executar `digitalWrite(PINO_RELE_CANAL_1, LOW)` imediatamente na primeira linha da função para garantir desligamento imediato.
2. Configurar `pinMode(PINO_LED_STATUS, OUTPUT)` e desligar o LED (`digitalWrite(PINO_LED_STATUS, HIGH)`).
3. Montar LittleFS.
4. (Restante do boot: Rede, NTP, Servidor Web, MQTT).

### Motivação
No momento do boot, o pino GPIO 0 do ESP8266 é uma linha de *boot strapping* (deve estar em nível HIGH para iniciar a partir da memória Flash). Para evitar que ruídos transitórios ou chaveamento precoce do pino acionem a válvula solenoide antes de o microcontrolador carregar o firmware, o estado seguro `LOW` é forçado imediatamente.

---

# ADR-008 - Telemetria e Logs

### Decisão
Utilizar a porta Serial nativa do ESP8266 (`Serial.print` via pino TX/GPIO1 a 115200 bps) formatando a saída com níveis de severidade `[INFO]`, `[WARN]`, `[ERROR]`. Em caso de falha no parser JSON do LittleFS, registrar detalhadamente o nó que falhou.

### Motivação
Permitir depuração em bancada durante a gravação/desenvolvimento via conversor USB-TTL e diagnósticos rápidos de falhas em arquivos de configuração.

---

# ADR-009 - Mecanismo de Atualização Remota (OTA)

### Decisão
Implementar a atualização remota (OTA - *Over-The-Air*) baseada em interface web local via biblioteca nativa `ESP8266HTTPUpdateServer` mapeada na rota `/update`.

### Motivação
No ESP-01S, que não possui conector USB nativo integrado e requer adaptador externo para gravação cabeada, o recurso OTA torna-se indispensável para a manutenção do firmware em campo. A rota web `/update` permite atualizar o arquivo `.bin` compilado sem necessidade de reprogramador físico.

---

# ADR-010 - Protocolo MQTT para Telemetria e Controle Remoto

### Decisão
Utilizar a biblioteca `PubSubClient` para realizar a conexão MQTT com um broker externo de forma não-bloqueante. O dispositivo publica periodicamente telemetria no formato JSON:
`{"valvula": "LIGADA"|"DESLIGADA"}` (acrescido de métricas operacionais de uptime, wifi e memória livre).

Comandos aceitos:
* `LIGAR`: Aciona a válvula manualmente (com limite de 10 min).
* `DESLIGAR`: Desliga a válvula imediatamente.
* `AGENDAMENTOS`: Solicita a publicação das 6 sessões no tópico dedicado.
* `REBOOT`: Reinicia o microcontrolador com segurança.
* Inscrição no tópico `<mqttSubTopic>/agendar`: Cadastramento/atualização de agendamentos via payload JSON.

### Motivação
Comunicação assíncrona, leve e padronizada para integração com plataformas de automação (Home Assistant, Node-RED) e aplicativos móveis.

---

# ADR-011 - Proteção Térmica da Válvula Solenoide (Fracionamento e Limite Manual)

### Decisão
Para proteger fisicamente a válvula solenoide contra superaquecimento:
1. **Sessões Agendadas Longas (> 10 min):** O firmware irá fracionar a irrigação automaticamente em ciclos periódicos de 5 minutos ligado e 10 minutos desligado (período total de 15 minutos do ciclo). A janela total da sessão é expandida (esticada) para garantir que o tempo útil configurado seja integralmente cumprido com a válvula aberta.
2. **Controle Manual:** Qualquer ativação manual do relé (via web ou MQTT) terá um temporizador rígido que desligará o dispositivo automaticamente após 10 minutos contínuos.

### Motivação
Evitar danos térmicos à solenoide decorrentes de acionamento contínuo prolongado.

---

# ADR-012 - Autovigilância Anti-Ruído, Diagnóstico de Boot e Tópicos MQTT Específicos

### Decisão
1. **Auto-verificação Anti-Ruído (Hardware Safety Check):** O motor de agendamento inspeciona o estado lógico do pino **GPIO 0**. Caso o pino físico seja encontrado em nível alto (`HIGH`) sem autorização explícita (sem agendamento ativo e sem override manual), o firmware desliga o pino instantaneamente (`LOW`), incrementa o contador de disparos espúrios e gera logs de alerta.
2. **Diagnóstico de Boot, Agendamento via JSON e Tópicos MQTT Dedicados:**
   - Na inicialização e reconexão MQTT, publica a razão de reset de hardware (`ESP.getResetReason()`), tempo online e disparos espúrios no tópico `<mqttPubTopic>/reboot`.
   - Publica as 6 sessões de irrigação ativas no tópico `<mqttPubTopic>/agendamentos`.
   - Permite o cadastramento de novos agendamentos via payload JSON no tópico de assinatura ou `<mqttSubTopic>/agendar`.

### Motivação
Garantir resiliência contra surtos eletromagnéticos e registrar telemetrias de estabilidade física do nó.

---

# ADR-013 - Auto-Recuperação Periódica de Conexão Wi-Fi STA em Modo AP/Dual Mode

### Decisão
Em caso de perda da conexão com o roteador local:
1. O firmware ativa o modo dual/AP de fallback (`WIFI_AP_STA`), garantindo que o Access Point local (`192.168.4.1`) permaneça acessível.
2. O sistema executa chamadas de reconexão não-bloqueantes (`WiFi.begin(...)`) periodicamente a cada 20 segundos via controle de tempo `millis()`.
3. Assim que a rede do roteador for restaurada, o dispositivo desativa o AP de fallback e retorna ao modo estrito `WIFI_STA`.

### Motivação
Evitar que quedas de roteador deixem o equipamento permanentemente desconectado da automação.

---

# ADR-014 - Migração para ESP-01S e Descontinuação do Sensor de Fluxo

### Decisão
Adaptar a arquitetura física do projeto para o módulo **ESP-01S** integrado diretamente a uma placa de relé dedicada. Como o ESP-01S disponibiliza apenas dois pinos de uso geral acessíveis (GPIO 0 e GPIO 2) além das linhas seriais de gravação (GPIO 1/TX e GPIO 3/RX):
1. O pino **GPIO 0** é dedicado ao acionamento do relé da válvula solenoide.
2. O pino **GPIO 2** é dedicado ao LED azul indicador de status de rede.
3. O sensor de fluxo de pulsos Hall YF-S201B (anteriormente no GPIO 14 do NodeMCU) é integralmente removido do firmware, das rotinas de interrupção, dos arquivos de persistência e das páginas web.

### Motivação
Permitir a construção de um atuador de irrigação ultracompacto e de baixo custo utilizando o popular conjunto ESP-01S + módulo relé, adequando a lógica de negócio às restrições estritas de pinagem do componente.
