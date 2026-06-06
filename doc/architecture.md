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
  "ssidSta": "RedeLocalCasa",
  "pwdSta": "senhaSuperSecreta",
  "ntpServer": "pool.ntp.br",
  "timeZone": -3,
  "ntpSyncInterval": 21600
}
```

(Nota: ntpSyncInterval define o tempo em segundos entre sincronizações - ex: 21600s = 6 horas).

**2. `/irrigation.json`**

```json
{
  "allowSimultaneous": false,
  "channel1": [ { "enabled": true, "start": "06:00", "duration": 10 } ],
  "channel2": [ { "enabled": false, "start": "18:00", "duration": 15 } ]
}
``` 

### Motivação
Isolamento de domínio. Atualizar horários de irrigação não exige tocar no arquivo de rede, reduzindo o desgaste da memória Flash e o risco de corrupção cruzada.

---

# ADR-005 - Motor de Agendamento (Scheduler Customizado)

### Decisão
Não utilizar bibliotecas de alarmes baseadas em interrupções ou callbacks (como TimeAlarms).
O agendamento será implementado através de um Super Loop (polling) que compara o horário atual gerado pelo NTPClient com os timestamps ou equivalentes numéricos (ex: minutos desde a meia-noite) das sessões ativas no /irrigation.json.

### Motivação
Evitar race conditions com processos assíncronos do ESP8266 (WiFi e WebServer).

Maior previsibilidade de execução.

Facilidade para ignorar "regas perdidas" durante quedas de energia (o loop simplesmente avalia: "o horário atual está dentro de uma janela de rega ativa?").

# ADR-006 - Responsabilidade de Validação de Conflitos
### Decisão
A validação de conflito de horários (quando allowSimultaneous é falso) será feita obrigatoriamente no Backend (Firmware em C++). O frontend pode ter validações visuais de conveniência, mas o Endpoint HTTP que recebe o POST/PUT do JSON rejeitará cargas úteis (HTTP 400 Bad Request) se houver sobreposição de minutos.

### Motivação
A segurança do equipamento (risco de falta de pressão d'água ou sobrecarga elétrica) não pode depender de código rodando no navegador do cliente, que pode ser burlado ou falhar silenciosamente.

# ADR-007 - Controle Físico e Sequência de Inicialização
### Decisão
Os módulos de relé operam em lógica Active High.
A sequência de inicialização no setup() deve ser estritamente:

Configurar pinMode para os GPIOs dos relés.

Escrever LOW imediata e explicitamente em todos os canais.

Montar LittleFS.

(Restante do boot: Rede, NTP, Servidor Web).

### Motivação
Garantir que picos de flutuação de pino durante o boot do ESP8266 não acionem as válvulas de água indevidamente antes que o sistema tenha consciência de seu próprio estado.

# ADR-008 - Telemetria e Logs
### Decisão
Utilizar a porta Serial nativa do ESP8266 (Serial.print) formatando a saída com níveis de severidade [INFO], [WARN], [ERROR]. Em caso de falha no parser JSON do LittleFS, registrar detalhadamente o nó que falhou.

### Motivação
Permitir depuração em bancada (USB) durante o desenvolvimento e diagnósticos rápidos de falha estrutural de arquivos de configuração.
