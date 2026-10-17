# Especificação do Sistema IrrigaSolo

Projeto de sistema de irrigação automatizado baseado em agendamento de horários, controle via interface web local e monitoramento/controle remoto via protocolo MQTT na internet, utilizando microcontrolador com conectividade Wi-Fi. O nome **IrrigaSolo** reflete seu duplo propósito: cuidar do solo e operar de forma autônoma (solo).

## 1. Componentes de Hardware

* **Microcontrolador:** ESP8266 ESP-01S (1MB de memória Flash SPI).
* **Módulo Relé Integrado:** Módulo relé de 1 canal v4.0 com transistor NPN de chaveamento — acionado pelo pino **GPIO 0** em nível lógico **ALTO (Active HIGH)**.
* **Válvula Solenoide:** 1 Válvula solenoide simples (tipo máquina de lavar roupas — 220V/127V).
* **LED Indicador de Status:** LED onboard azul do ESP-01S — conectado ao **GPIO 2** em nível lógico **BAIXO (Active LOW)**.
* **Alimentação:** Fonte de alimentação 5V DC (alimentando o módulo de relé e o regulador LDO de 3.3V que fornece energia ao ESP-01S).

*Nota sobre o Sensor de Fluxo:* Devido à severa restrição de pinagem física do ESP-01S (que expõe apenas GPIO 0, GPIO 2, TX e RX), o sensor de fluxo de água Hall YF-S201B presente em protótipos anteriores com NodeMCU foi **removido** do projeto.

---

## 2. Requisitos Funcionais

### RF1 - Agendamento e Acionamento

O sistema deve permitir o agendamento de sessões de irrigação para o canal único de irrigação.

* **RF1.1:** O canal suporta até 6 sessões de irrigação por dia (início em HH:MM e duração em minutos).
  * Se a duração de uma sessão for menor ou igual a 10 minutos, o relé operará de forma contínua.
  * Se a duração for superior a 10 minutos, o acionamento será fracionado para resfriamento térmico da solenoide em ciclos de 5 minutos ligado e 10 minutos desligado (período total de 15 minutos do ciclo).
  * Para garantir que a duração configurada reflita o tempo real de irrigação (água ligada), a janela total de tempo decorrido da sessão será expandida (esticada) proporcionalmente aos períodos de descanso.
* **RF1.2:** A base de tempo deve ser sincronizada via NTP, com consultas periódicas. Em caso de falha de rede, o sistema mantém o relógio local.
* **RF1.3:** Recuperação de Falha de Energia: O sistema não deve acumular ou repor regas perdidas durante quedas de energia. Ao reiniciar, aguarda-se o próximo horário programado validado pelo relógio atual.
* **RF1.4:** Proteção no Acionamento Manual: Para proteger fisicamente a bobina da solenoide, qualquer acionamento manual de válvula (via interface web ou comando MQTT) é limitado a no máximo 10 minutos contínuos, desligando-se automaticamente após esse período.

### RF2 - Gerenciamento de Rede e Configuração Inicial

O dispositivo deve gerenciar sua conexão Wi-Fi de forma autônoma.

* **RF2.1:** Na ausência de rede conhecida, opera em modo Access Point (SSID padrão: `IrrigaSolo`, Senha: `admin123`) e exibe um Portal Cativo para configuração.
* **RF2.2:** A interface inicial deve coletar: SSID e Senha da rede local, URL do servidor NTP (padrão `pool.ntp.br`), Fuso Horário, Intervalo de Sincronização e os parâmetros de conexão MQTT (Broker, Porta, Usuário, Senha, Client ID, Tópico de Publicação, Tópico de Assinatura e Intervalo de Publicação).
* **RF2.3:** Em caso de perda de conexão STA (rede local indisponível/reboot do roteador), o sistema deve acionar o fallback para o modo dual/AP (permitindo reconfiguração sem interromper os agendamentos), mantendo tentativas periódicas não-bloqueantes de reconexão ao SSID configurado a cada 20 segundos. Assim que a rede for restaurada, o dispositivo retorna automaticamente ao modo STA pleno.

### RF3 - [REMOVIDO] Medição de Fluxo de Água

*(Descontinuado para a plataforma ESP-01S)* Devido à restrição de pinagem física do ESP-01S, que possui apenas 2 GPIOs de uso geral (GPIO 0 para o relé e GPIO 2 para o LED de status), o sensor de fluxo de pulsos Hall YF-S201B não é suportado nesta variante de hardware. O controle do consumo é baseado estritamente na gestão do tempo de acionamento da válvula solenoide.

### RF4 - Interface Web (Painel de Controle)

A interface deve ser acessível via navegador na rede local.

* **RF4.1:** **Painel Principal:** Exibe status do Wi-Fi, conexão MQTT, hora atual do sistema, estado da válvula e controle manual (Ligar/Desligar) que não interfere na programação automática.
* **RF4.2:** **Painel de Horários:** Interface para cadastrar, editar, habilitar ou desabilitar as 6 sessões do canal de irrigação.
* **RF4.3:** **Painel de Configuração:** Interface para alterar credenciais de rede, NTP e as configurações do Broker MQTT (incluindo Client ID, tópicos e intervalo de publicação).

### RF5 - Comunicação MQTT

O sistema deve permitir monitoramento e controle através de um broker local ou na internet utilizando o protocolo MQTT.

* **RF5.1:** **Publicação de Telemetria:** O dispositivo deve publicar periodicamente, com intervalo configurável via web (padrão: 10 segundos, mínimo de 1s), o estado atual no tópico de publicação configurado via web no formato JSON:
  `{"valvula": "LIGADA"|"DESLIGADA"}` (complementado com diagnósticos de telemetria como uptime, memória livre, rssi, ip e firmware).
* **RF5.2:** **Recepção de Comandos e Agendamentos via MQTT:** O dispositivo deve se inscrever no tópico de assinatura configurado via web e aceitar comandos para ligar/desligar a válvula manualmente (payload `LIGAR` ou `DESLIGAR`), solicitar agendamentos (`AGENDAMENTOS`), reiniciar o dispositivo (`REBOOT`) e **cadastrar/atualizar agendamentos via payload JSON** (ex: `{"sessao":0, "habilitada":true, "inicio":"06:30", "duracao":15}`). Ao receber um novo agendamento via MQTT, o firmware deve atualizar a estrutura em memória, persistir a alteração no LittleFS (`/irrigation.json`) e publicar a confirmação.
* **RF5.3:** **Tópicos Dedicados de Reboot, Agendamentos e Inscrição de Agendar:** O dispositivo deve publicar automaticamente mensagens em tópicos dedicados ao estabelecer a conexão MQTT:
  * Tópico de Reboot (`<mqttPubTopic>/reboot`): Publica payload JSON com motivo de reinício (`motivo_reset`), uptime anterior e contagem de disparos espúrios.
  * Tópico de Agendamentos (`<mqttPubTopic>/agendamentos`): Publica payload JSON com as 6 sessões de irrigação cadastradas.
  * Tópico de Inscrição de Agendamento (`<mqttSubTopic>/agendar`): Opcional/dedicado para receber agendamentos via JSON.

### RF6 - Resiliência e Tolerância a Falhas

O sistema deve operar de forma segura mesmo em cenários de corrupção de dados ou interferências elétricas.

* **RF6.1:** Falha em configurações de rede ou MQTT: Assume valores AP padrão e aguarda intervenção.
* **RF6.2:** Falha em configurações de irrigação: Mantém o relé desligado, inibe acionamentos automáticos e registra o erro, mantendo a interface web acessível para reconfiguração.
* **RF6.3:** Proteção Anti-Ruído e Auto-Verificação do Pino da Válvula: O firmware deve verificar continuamente se o estado físico do relé (GPIO 0) é condizente com as permissões atuais (agendamento ativo ou controle manual). Se um pico de interferência eletromagnética (ruído elétrico) acionar a válvula indevidamente, o sistema deve desligá-la imediatamente (em milissegundos), restaurando o nível LOW, incrementando o contador de disparos espúrios e registrando o evento.

### RF7 - Atualização de Firmware Sem Fios (OTA)

O sistema deve permitir a atualização de seu código-fonte remotamente pela rede sem necessidade de conexão física USB.

* **RF7.1:** O firmware deve integrar um servidor de atualização HTTP acessível na rota `/update`.
* **RF7.2:** A atualização deve aceitar arquivos binários compilados (`.bin`) enviados diretamente via formulário web no navegador.

---

## 3. Critérios de Aceitação (CA)

* **CA-01:** O relé (GPIO 0) deve iniciar estritamente desligado (nível LOW) no momento em que o equipamento é energizado, antes de qualquer inicialização de rádio ou sistema de arquivos.
* **CA-02:** O canal de irrigação liga no exato minuto programado e desliga automaticamente após decorrer a duração configurada. Horários desabilitados são ignorados.
* **CA-03:** O LED de status onboard no GPIO 2 deve sinalizar visualmente o estado da rede Wi-Fi (piscando em modo AP e aceso fixo em modo STA).
* **CA-04:** O sistema reconecta-se ao broker MQTT de forma assíncrona (não-bloqueante) em caso de perda de conexão e responde a comandos remotos em tempo real.
* **CA-05:** Alterações de rede, NTP, MQTT e agendamentos sobrevivem a reinicializações físicas do hardware através do LittleFS em flash de 1MB.
* **CA-06:** O upload de um binário válido na rota `/update` atualiza o firmware de forma automática e reinicia o ESP-01S com a nova versão ativa.
* **CA-07:** O acionamento de regas agendadas com duração maior que 10 minutos executa obrigatoriamente de forma fracionada (5 minutos ligado, 10 minutos desligado), com a janela de execução expandida (esticada) para garantir que a duração configurada seja o tempo real de irrigação (válvula aberta). Qualquer acionamento manual desliga automaticamente após 10 minutos contínuos.
* **CA-08:** O sistema detecta acionamentos espúrios provocados por ruído elétrico no GPIO 0 e desliga a válvula imediatamente, mantendo o contador de disparos espúrios atualizado.
* **CA-09:** Na inicialização e reconexão MQTT, o firmware publica o motivo do reset e a lista de agendamentos nos tópicos `<mqttPubTopic>/reboot` e `<mqttPubTopic>/agendamentos`.
