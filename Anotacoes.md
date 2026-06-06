# Guia de Estudos — Firmware IrrigaSolo 🌱

Este documento serve como um manual didático detalhado para estudantes de tecnologia e sistemas embarcados. Ele descreve a arquitetura geral do firmware **IrrigaSolo** e detalha o funcionamento conceitual e prático de cada um de seus cinco módulos fundamentais.

---

## 1. Descrição Geral do Projeto

O **IrrigaSolo** é um firmware para microcontroladores baseado no chip **ESP8266** (NodeMCU), projetado para controlar a irrigação de jardins através de 2 canais independentes (relés ativando válvulas solenoides). Suas principais premissas de projeto são:
* **Autonomia Temporal:** Sincronização do relógio via NTP com fallback para oscilador interno caso a internet caia.
* **Segurança e Confiabilidade:** Pinos de hardware são forçados a estados seguros no instante do boot e no caso de arquivos de configuração corrompidos.
* **Persistência Local Dedicada:** Armazenamento em arquivos JSON no LittleFS separados por domínio de interesse (Rede vs. Irrigação) para prevenir desgaste prematuro e corrupção cruzada.
* **Interface Web Embutida Leve:** A interface de controle é guardada na memória flash do controlador (`PROGMEM`) em formato compactado e as alterações de estado são feitas de modo assíncrono via chamadas de API JSON (Fetch API), evitando o recarregamento total de páginas.
* **Arquitetura Baseada em Super Loop Não-Bloqueante:** Proibição do uso de `delay()` para garantir que o Watchdog Timer (WDT) não reinicie a placa e as pilhas de comunicação Wi-Fi operem com fluidez.

---

## 2. Diagrama de Fluxo Geral (Inicialização e Loop)

O fluxo principal do firmware localiza-se no arquivo [main.cpp](file:///home/rpb/Documentos/PlatformIO/Projects/IrrigacaoJardins/src/main.cpp).

```mermaid
flowchart TD
    Start([Início / Boot]) --> InitReles[1. inicializarReles: GPIO LOW Imediato]
    InitReles --> InitSerial[2. Serial.begin: Telemetria]
    InitSerial --> InitFS[3. inicializarLittleFS: Monta Flash]
    InitFS --> LoadConfigs[4. carregarConfiguracao: Rede & Irrigação]
    LoadConfigs --> InitWifi[5. inicializarWifi: Conecta STA ou inicia AP]
    InitWifi --> InitNtp[6. inicializarNtp: Sync Tempo]
    InitNtp --> InitServer[7. inicializarServidor: Porta 80]
    InitServer --> Loop[Início do Super Loop]
    Loop --> ProcessReq[processarRequisicoes: Trata HTTP Client]
    ProcessReq --> CheckWifi[verificarConexaoWifi: Monitora STA e AP Fallback]
    CheckWifi --> UpdateNtp[atualizarNtp: Atualiza Relógio]
    UpdateNtp --> PollingTime{Passou 1 segundo?}
    PollingTime -- Sim --> EvalSched[avaliarAgendamentos: Verifica Relés]
    PollingTime -- Não --> Loop
    EvalSched --> Loop
```

---

## 3. Detalhamento Técnico dos Módulos

### 3.1. Módulo de Persistência (`persistencia.h` / `persistencia.cpp`)
* **Conceito Abstrato:** Este módulo abstrai a leitura e escrita de dados no sistema de arquivos **LittleFS**. Ele converte estruturas em C/C++ para strings JSON e vice-versa, utilizando a biblioteca **ArduinoJson (v7)**. Para evitar alocações dinâmicas na memória RAM (evitando fragmentação e crashes), utiliza buffers estáticos e otimizados. 
* **Lógica de Fail-Safe:** Se o arquivo de configuração estiver ausente ou corrompido, o firmware aplica configurações padrão seguras imediatamente (relés desligados e modo Wi-Fi em Access Point para possibilitar reconfiguração).

**Demonstração em Código:**
O método a seguir mostra como as sessões do Canal 1 são lidas de `/irrigation.json` e como o WDT é alimentado usando `yield()` ao processar grandes arrays JSON em hardware embarcado:

```cpp
// Trecho extraído de src/persistencia.cpp
// Deserializar sessões do Canal 1
JsonArray arrayCh1 = documento["channel1"].as<JsonArray>();
for (uint8_t i = 0; i < MAX_SESSOES_POR_CANAL; i++) {
    if (arrayCh1 && i < arrayCh1.size()) {
        JsonObject sessao = arrayCh1[i];
        configIrrigacao.canal1[i].habilitada = sessao["enabled"] | false;
        strlcpy(configIrrigacao.canal1[i].inicio,
                sessao["start"] | "00:00",
                sizeof(configIrrigacao.canal1[i].inicio));
        configIrrigacao.canal1[i].duracao = sessao["duration"] | 0;
    } else {
        // Slots não definidos no JSON ficam desabilitados
        configIrrigacao.canal1[i].habilitada = false;
        strlcpy(configIrrigacao.canal1[i].inicio, "00:00",
                sizeof(configIrrigacao.canal1[i].inicio));
        configIrrigacao.canal1[i].duracao = 0;
    }
    yield(); // Alimentar o WDT durante parsing de arrays (restrição embarcada)
}
```

---

### 3.2. Módulo de Rede Wi-Fi (`redeWifi.h` / `redeWifi.cpp`)
* **Conceito Abstrato:** Gerencia a conexão Wi-Fi nos modos **STA** (Estação, conectado ao roteador da casa) e **AP** (Access Point, criando uma rede própria do IrrigaSolo para configuração). O LED de status informa o estado atual: piscando se estiver em modo AP (aguardando rede) ou aceso se estiver conectado à rede local em modo STA.
* **Decisão Arquitetural Importante (ADR-003):** O firmware chama `WiFi.persistent(false)` para desabilitar o cache automático de credenciais na Flash nativo do SDK do ESP8266. Isso preserva a Flash e torna os arquivos JSON locais a única fonte de verdade.
* **Timeout Não-bloqueante e Fallback (RF2.3):** A tentativa de conexão na rede local tem um limite de tempo. Se falhar, o dispositivo gera seu próprio AP de forma não-bloqueante sem congelar a execução do sistema de irrigação.

**Demonstração em Código:**
Abaixo está o loop de conexão com timeout não-bloqueante que protege o Watchdog Timer usando `yield()`:

```cpp
// Trecho extraído de src/redeWifi.cpp
static bool tentarConexaoSta() {
    Serial.print(F("[INFO] Tentando conectar à rede: "));
    Serial.println(configRedeLocal.ssidSta);

    WiFi.mode(WIFI_STA);
    WiFi.begin(configRedeLocal.ssidSta, configRedeLocal.senhaSta);

    unsigned long inicioTentativa = millis();

    while (WiFi.status() != WL_CONNECTED) {
        // Timeout atingido — conexão falhou
        if (millis() - inicioTentativa >= TIMEOUT_CONEXAO_STA_MS) {
            Serial.println(F("\n[WARN] Timeout de conexão STA atingido."));
            return false;
        }
        yield(); // Alimenta o WDT e processa pilhas de rede
        delay(100); // Pequeno intervalo para não saturar o loop
    }
    
    modoApAtivo = false;
    // ...
    return true;
}
```

---

### 3.3. Módulo de Sincronização NTP (`relogioNtp.h` / `relogioNtp.cpp`)
* **Conceito Abstrato:** Utiliza o protocolo NTP (*Network Time Protocol*) para sincronizar o relógio interno do ESP8266. O fuso horário e a frequência de sincronização são parametrizáveis.
* **Funcionamento com Falha de Conectividade:** Caso a conexão à rede local caia, o objeto `NTPClient` estima o tempo de forma autônoma baseando-se no contador interno de milissegundos do chip (`millis()`). A precisão sofre um desvio sutil com o tempo (*drift*), mas o sistema continua operando a rega normalmente.

**Demonstração em Código:**
Conversão de data/hora a partir do Epoch UTC sincronizado, aplicando fuso horário e gerando string em Português:

```cpp
// Trecho extraído de src/relogioNtp.cpp
String obterDataFormatada() {
    unsigned long epochTime = clienteNtp.getEpochTime();
    time_t rawTime = (time_t)epochTime;
    struct tm* timeInfo = gmtime(&rawTime);

    char bufferData[32];
    snprintf(bufferData, sizeof(bufferData), "%02d de %s de %d",
             timeInfo->tm_mday,
             NOMES_MESES[timeInfo->tm_mon], // tm_mon: 0–11
             timeInfo->tm_year + 1900);

    return String(bufferData);
}
```

---

### 3.4. Módulo Agendador (`agendador.h` / `agendador.cpp`)
* **Conceito Abstrato:** É o motor de regras que decide se um relé deve ligar ou desligar. O tempo é avaliado como "minutos desde meia-noite" (um valor de 0 a 1439). Cada sessão programada cria uma **janela de irrigação** dada por `[minutoInicio, minutoInicio + duracao)`. O agendador lê as sessões ativas e verifica se o minuto atual se enquadra nelas.
* **Validação de Conflitos (ADR-006):** Para evitar que os dois canais operem juntos e reduzam a pressão da água (quando a simultaneidade é proibida), o firmware executa uma validação geométrica de intervalos cruzados antes de salvar os dados.

**Diagrama de Validação de Sobreposição:**
```
Janela 1 (Canal 1): [a ----------------- b)
Janela 2 (Canal 2):           [c --------------- d)
Há sobreposição se: a < d  E  c < b  (Resulta em Conflito!)
```

**Demonstração em Código:**
Abaixo está a função de validação geométrica das janelas de rega e a avaliação periódica no loop:

```cpp
// Trecho extraído de src/agendador.cpp
static bool janelasSeOverpoem(int inicio1, uint8_t duracao1, int inicio2, uint8_t duracao2) {
    int fim1 = inicio1 + duracao1;
    int fim2 = inicio2 + duracao2;

    // Duas janelas [a, b) e [c, d) se sobrepõem se a < d && c < b
    return (inicio1 < fim2) && (inicio2 < fim1);
}
```

```cpp
// Trecho extraído de src/agendador.cpp (avaliação de canais no loop)
if (minutoAtual >= minutosInicio && minutoAtual < (minutosInicio + sessao.duracao)) {
    canal1DeveEstarLigado = true;
    break; // Basta uma sessão ativa para ligar o canal
}
```

---

### 3.5. Módulo Servidor Web (`servidorWeb.h` / `servidorWeb.cpp`)
* **Conceito Abstrato:** Cria um servidor HTTP ouvindo na porta 80. Ele serve as páginas HTML embutidas na memória Flash e disponibiliza rotas de API REST leves que devolvem e aceitam JSON.
* **Minimização de Uso da RAM:** Em vez de manter grandes strings HTML em memória RAM, o firmware utiliza o modificador `PROGMEM` (`servidor.send_P`), fazendo com que o microcontrolador envie os dados direto da Flash do dispositivo para a rede.

**Demonstração em Código:**
Declaração de endpoints HTTP e envio de páginas a partir da memória de programa (`PROGMEM`):

```cpp
// Trecho extraído de src/servidorWeb.cpp
void inicializarServidor(ConfiguracaoRede* configRede, ConfiguracaoIrrigacao* configIrrigacao) {
    ptrConfigRede      = configRede;
    ptrConfigIrrigacao = configIrrigacao;

    // Páginas HTML (servidas de PROGMEM)
    servidor.on("/",         HTTP_GET,  handlePaginaIndex);
    servidor.on("/horarios", HTTP_GET,  handlePaginaHorarios);
    servidor.on("/config",   HTTP_GET,  handlePaginaConfig);

    // Endpoints de API REST
    servidor.on("/api/status",  HTTP_GET,  handleApiStatus);
    servidor.on("/api/control", HTTP_POST, handleApiControl);
    servidor.on("/api/schedules", HTTP_POST, handleApiPostSchedules);
    // ...
    servidor.begin();
}
```

---

## 4. Questões Rápidas de Revisão (Para a Prova 📝)

1. **Por que não devemos utilizar `delay()` no `loop()` principal de um firmware IoT?**
   * *Resposta:* O `delay()` é bloqueante. Ele impede que as pilhas de rede Wi-Fi e os clientes HTTP sejam processados, além de impedir o disparo correto do Watchdog Timer (WDT), resultando em travamento ou reset do hardware.
2. **O que acontece se o arquivo `/irrigation.json` estiver corrompido?**
   * *Resposta:* O módulo de persistência intercepta o erro de desserialização (JSON inválido), aplica defaults seguros (todas as sessões desabilitadas, relés inativos) e impede acionamentos incorretos no boot do sistema.
3. **Qual é o papel da instrução `WiFi.persistent(false)`?**
   * *Resposta:* Ela instrui o SDK do ESP8266 a não gravar dados de conexão Wi-Fi na Flash interna a cada chamada de boot do rádio, evitando o desgaste prematuro da memória Flash e mantendo o LittleFS como única fonte de verdade.
