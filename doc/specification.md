# Especificação do Sistema IrrigaSolo

Projeto de sistema de irrigação automatizado baseado em agendamento de horários, controle via interface web local e monitoramento/controle remoto via protocolo MQTT na internet, utilizando microcontrolador com conectividade Wi-Fi. O nome **IrrigaSolo** reflete seu duplo propósito: cuidar do solo e operar de forma autônoma (solo).

## 1. Componentes de Hardware

* NodeMCU ESP-12E (v0.9).
* Módulo relé de 1 canal (com isolamento optoacoplado).
* 1 Válvula solenoide simples (tipo máquina de lavar roupas - 220V).
* 1 Sensor de fluxo de água Hall YF-S201B (3 fios: VCC, GND, Sinal).
* Fonte de alimentação 5V (adequada para o NodeMCU e picos de corrente da bobina do relé).

*Nota: Por hora, ignore quaisquer imagens de circuitos que existam neste diretório.*

## 2. Requisitos Funcionais

### RF1 - Agendamento e Acionamento
O sistema deve permitir o agendamento de sessões de irrigação para o canal único de irrigação.
* **RF1.1:** O canal suporta até 6 sessões de irrigação por dia (início em HH:MM e duração em minutos).
* **RF1.2:** A base de tempo deve ser sincronizada via NTP, com consultas periódicas. Em caso de falha de rede, o sistema mantém o relógio local.
* **RF1.3:** Recuperação de Falha de Energia: O sistema não deve acumular ou repor regas perdidas durante quedas de energia. Ao reiniciar, aguarda-se o próximo horário programado validado pelo relógio atual.

### RF2 - Gerenciamento de Rede e Configuração Inicial
O dispositivo deve gerenciar sua conexão Wi-Fi de forma autônoma.
* **RF2.1:** Na ausência de rede conhecida, opera em modo Access Point (SSID padrão: `IrrigaSolo`, Senha: `admin123`) e exibe um Portal Cativo para configuração.
* **RF2.2:** A interface inicial deve coletar: SSID e Senha da rede local, URL do servidor NTP (padrão `pool.ntp.br`), Fuso Horário, Intervalo de Sincronização e os parâmetros de conexão MQTT (Broker, Porta, Usuário, Senha, Client ID, Tópico de Publicação e Tópico de Assinatura).
* **RF2.3:** Em caso de perda de conexão STA (rede local indisponível), o sistema deve acionar fallback para o modo AP, permitindo nova configuração sem interromper os agendamentos já carregados.

### RF3 - Medição de Fluxo de Água
O sistema deve integrar o sensor de fluxo YF-S201B para monitorar o consumo hídrico.
* **RF3.1:** O firmware deve calcular a vazão instantânea de água em litros por minuto (L/min).
* **RF3.2:** O sistema deve acumular o volume total de água consumido (em litros) durante o acionamento da válvula.
* **RF3.3:** O volume total acumulado pode ser zerado via interface web local ou comando MQTT.

### RF4 - Interface Web (Painel de Controle)
A interface deve ser acessível via navegador na rede local.
* **RF4.1:** **Painel Principal:** Exibe status do Wi-Fi, conexão MQTT, hora atual do sistema, estado da válvula e medições em tempo real do sensor de fluxo (vazão e volume total). Permite acionamento manual (Ligar/Desligar) que não interfere na programação automática.
* **RF4.2:** **Painel de Horários:** Interface para cadastrar, editar, habilitar ou desabilitar as 6 sessões do canal de irrigação.
* **RF4.3:** **Painel de Configuração:** Interface para alterar credenciais de rede, NTP e as configurações do Broker MQTT (incluindo Client ID e tópicos).

### RF5 - Comunicação MQTT
O sistema deve permitir monitoramento e controle através de um broker local ou na internet utilizando o protocolo MQTT.
* **RF5.1:** **Publicação de Telemetria:** O dispositivo deve publicar periodicamente (ex: a cada 5 segundos) o estado atual no tópico de publicação configurado via web no formato JSON:
  `{"valvula": "LIGADA"|"DESLIGADA", "vazao": 0.0, "volume": 0.0}`
* **RF5.2:** **Recepção de Comandos:** O dispositivo deve se inscrever no tópico de assinatura configurado via web e aceitar comandos para ligar/desligar a válvula manualmente (ex: payload `LIGAR` ou `DESLIGAR`) e zerar o volume acumulado (ex: payload `ZERAR`).

### RF6 - Resiliência e Tolerância a Falhas
O sistema deve operar de forma segura mesmo em cenários de corrupção de dados.
* **RF6.1:** Falha em configurações de rede ou MQTT: Assume valores AP padrão e aguarda intervenção.
* **RF6.2:** Falha em configurações de irrigação: Mantém o relé desligado, inibe acionamentos automáticos e registra o erro, mantendo a interface web acessível para reconfiguração.

### RF7 - Atualização de Firmware Sem Fios (OTA)
O sistema deve permitir a atualização de seu código-fonte remotamente pela rede sem necessidade de conexão física USB.
* **RF7.1:** O firmware deve integrar um servidor de atualização HTTP acessível na rota `/update`.
* **RF7.2:** A atualização deve aceitar arquivos binários compilados (`.bin`) enviados diretamente via formulário web no navegador.

## 3. Critérios de Aceitação (CA)

* **CA-01:** O relé deve iniciar estritamente desligado no momento em que o equipamento é energizado.
* **CA-02:** O canal de irrigação liga no exato minuto programado e desliga automaticamente após decorrer a duração configurada. Horários desabilitados são ignorados.
* **CA-03:** O sistema mede a vazão instantânea e acumula o volume de água corretamente ao receber pulsos no pino de interrupção.
* **CA-04:** O sistema reconecta-se ao broker MQTT de forma assíncrona (não-bloqueante) em caso de perda de conexão e responde a comandos remotos em tempo real.
* **CA-05:** Alterações de rede, NTP, MQTT e agendamentos sobrevivem a reinicializações físicas do hardware.
* **CA-06:** O upload de um binário válido na rota `/update` atualiza o firmware de forma automática e reinicia o microcontrolador com a nova versão ativa.