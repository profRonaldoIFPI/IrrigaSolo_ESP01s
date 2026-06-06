# Especificação do Sistema IrrigaSolo

Projeto de sistema de irrigação automatizado baseado em agendamento de horários e controle via interface web local, utilizando microcontrolador com conectividade Wi-Fi. O nome **IrrigaSolo** reflete seu duplo propósito: cuidar do solo e operar de forma autônoma (solo).

## 1. Componentes de Hardware

* NodeMCU ESP-12E (v0.9).
* Módulo relé de 2 canais (com isolamento optoacoplado).
* 2 Válvulas solenoides simples (tipo máquina de lavar roupas - 220V).
* Fonte de alimentação 5V (adequada para o NodeMCU e picos de corrente das bobinas dos relés).

## 2. Requisitos Funcionais

### RF1 - Agendamento e Acionamento
O sistema deve permitir o agendamento independente para dois canais de irrigação.
* **RF1.1:** Cada canal suporta até 6 sessões de irrigação por dia (início em HH:MM e duração em minutos).
* **RF1.2:** A base de tempo deve ser sincronizada via NTP, com consultas periódicas. Em caso de falha de rede, o sistema mantém o relógio local.
* **RF1.3:** Recuperação de Falha de Energia: O sistema não deve acumular ou repor regas perdidas durante quedas de energia. Ao reiniciar, aguarda-se o próximo horário programado validado pelo relógio atual.

### RF2 - Gerenciamento de Rede e Configuração Inicial
O dispositivo deve gerenciar sua conexão Wi-Fi de forma autônoma.
* **RF2.1:** Na ausência de rede conhecida, opera em modo Access Point (SSID padrão: `IrrigaSolo`, Senha: `admin123`) e exibe um Portal Cativo para configuração.
* **RF2.2:** A interface inicial deve coletar: SSID e Senha da rede local, URL do servidor NTP (padrão `pool.ntp.br`), Fuso Horário e Intervalo de Sincronização.
* **RF2.3:** Em caso de perda de conexão STA (rede local indisponível), o sistema deve acionar fallback para o modo AP, permitindo nova configuração sem interromper os agendamentos já carregados.

### RF3 - Gerenciamento de Conflitos e Simultaneidade
O sistema deve possuir uma trava de segurança contra baixa pressão de água, gerenciada pela opção de simultaneidade.
* **RF3.1:** Se a simultaneidade estiver desabilitada (padrão), o sistema deve recusar o salvamento de qualquer configuração de horários que gere sobreposição entre o Canal 1 e o Canal 2.
* **RF3.2:** A validação de conflitos deve ser garantida pelo sistema antes do salvamento efetivo, impedindo que acionamentos concorrentes ocorram por falha na interface do usuário.

### RF4 - Interface Web (Painel de Controle)
A interface deve ser acessível via navegador na rede local.
* **RF4.1:** **Painel Principal:** Exibe status do Wi-Fi, hora atual do sistema e estado em tempo real das válvulas. Deve permitir acionamento manual (Ligar/Desligar) que não interfira na programação automática.
* **RF4.2:** **Painel de Horários:** Interface para cadastrar, editar, habilitar ou desabilitar as 6 sessões de cada canal, além do seletor de simultaneidade.

### RF5 - Resiliência e Tolerância a Falhas
O sistema deve operar de forma segura mesmo em cenários de corrupção de dados.
* **RF5.1:** Falha em configurações de rede: Assume valores AP padrão e aguarda intervenção.
* **RF5.2:** Falha em configurações de irrigação: Mantém relés desligados, inibe acionamentos automáticos e registra o erro, mantendo a interface web acessível para reconfiguração.

## 3. Critérios de Aceitação (CA)

* **CA-01:** Todos os relés devem iniciar estritamente desligados no momento em que o equipamento é energizado.
* **CA-02:** Um canal habilitado liga no exato minuto programado e desliga automaticamente após decorrer a duração configurada. Horários desabilitados são ignorados.
* **CA-03:** O sistema impede o salvamento e exibe erro caso o usuário tente cadastrar horários sobrepostos com a simultaneidade desativada.
* **CA-04:** Alterações de rede e agendamentos sobrevivem a reinicializações físicas do hardware.