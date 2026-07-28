#pragma once

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="pt-br">

<head>
        <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <meta name="description" content="Painel de controle de irrigação IrrigaSolo.">
    <style>
        :root {
            --bg-color: #0f172a;
            --card-bg: rgba(30, 41, 59, 0.7);
            --border-color: rgba(255, 255, 255, 0.1);
            --text-primary: #f8fafc;
            --text-secondary: #94a3b8;
            --primary: #10b981;
            --primary-hover: #059669;
            --danger: #ef4444;
            --danger-hover: #dc2626;
            --shadow: 0 10px 15px -3px rgba(0, 0, 0, 0.3);
            --glow-green: 0 0 15px rgba(16, 185, 129, 0.4);
            --glow-red: 0 0 15px rgba(239, 68, 68, 0.4);
        }

        * {
            box-sizing: border-box;
            margin: 0;
            padding: 0;
        }

        body {
            font-family: 'Outfit', -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, Helvetica, Arial, sans-serif;
            background: radial-gradient(circle at top, #1e293b, #0f172a);
            color: var(--text-primary);
            min-height: 100vh;
            display: flex;
            flex-direction: column;
            align-items: center;
            padding: 20px;
        }

        .container {
            width: 100%;
            max-width: 600px;
            margin-top: 20px;
        }

        header {
            text-align: center;
            margin-bottom: 30px;
        }

        h1 {
            font-size: 2.2rem;
            font-weight: 700;
            background: linear-gradient(to right, #10b981, #34d399);
            -webkit-background-clip: text;
            -webkit-text-fill-color: transparent;
            margin-bottom: 5px;
        }

        .subtitle {
            color: var(--text-secondary);
            font-size: 0.95rem;
            letter-spacing: 0.5px;
        }

        .card {
            background: var(--card-bg);
            backdrop-filter: blur(12px);
            -webkit-backdrop-filter: blur(12px);
            border: 1px solid var(--border-color);
            border-radius: 16px;
            padding: 24px;
            margin-bottom: 20px;
            box-shadow: var(--shadow);
            transition: transform 0.2s ease, border-color 0.2s ease;
        }

        .card:hover {
            border-color: rgba(16, 185, 129, 0.3);
        }

        h2 {
            font-size: 1.25rem;
            margin-bottom: 15px;
            border-left: 4px solid var(--primary);
            padding-left: 10px;
            color: var(--text-primary);
        }

        .btn-submit {
            width: 100%;
            padding: 14px;
            background-color: var(--primary);
            color: #ffffff;
            border: none;
            border-radius: 8px;
            font-size: 1rem;
            font-weight: 600;
            cursor: pointer;
            transition: all 0.2s ease;
            margin-top: 10px;
        }

        .btn-submit:hover {
            background-color: var(--primary-hover);
            box-shadow: var(--glow-green);
            transform: translateY(-1px);
        }

        .btn-submit:active {
            transform: translateY(1px);
        }

        .btn-submit:disabled {
            background-color: #475569;
            cursor: not-allowed;
            transform: none;
            box-shadow: none;
        }

        /* Estilos dos botões de canais e status */
        .status-row {
            display: flex;
            justify-content: space-between;
            align-items: center;
            padding: 12px 0;
            border-bottom: 1px solid rgba(255, 255, 255, 0.05);
        }

        .status-row:last-child {
            border-bottom: none;
            padding-bottom: 0;
        }

        .status-label {
            color: var(--text-secondary);
            font-weight: 500;
        }

        .status-value {
            font-weight: 600;
            font-size: 1.1rem;
        }

        .status-badge {
            padding: 4px 10px;
            border-radius: 9999px;
            font-size: 0.8rem;
            font-weight: 600;
            text-transform: uppercase;
        }

        .badge-connected {
            background-color: rgba(16, 185, 129, 0.2);
            color: var(--primary);
            border: 1px solid rgba(16, 185, 129, 0.3);
        }

        .badge-disconnected {
            background-color: rgba(239, 68, 68, 0.2);
            color: var(--danger);
            border: 1px solid rgba(239, 68, 68, 0.3);
        }

        .datetime-display {
            text-align: center;
        }

        .time-large {
            font-size: 3rem;
            font-weight: 700;
            letter-spacing: -1px;
            color: var(--text-primary);
            text-shadow: 0 0 20px rgba(255, 255, 255, 0.1);
            line-height: 1.1;
        }

        .date-sub {
            color: var(--text-secondary);
            font-size: 1.1rem;
            margin-top: 5px;
        }

        .channel-control {
            display: flex;
            align-items: center;
            justify-content: space-between;
            margin-bottom: 15px;
            padding: 15px;
            border-radius: 12px;
            background: rgba(255, 255, 255, 0.02);
            border: 1px solid rgba(255, 255, 255, 0.05);
        }

        .channel-info {
            display: flex;
            flex-direction: column;
        }

        .channel-name {
            font-size: 1.1rem;
            font-weight: 600;
        }

        .channel-status {
            font-size: 0.85rem;
            margin-top: 3px;
        }

        .status-on {
            color: var(--primary);
            font-weight: 600;
            text-shadow: var(--glow-green);
        }

        .status-off {
            color: var(--text-secondary);
        }

        .btn-group {
            display: flex;
            gap: 8px;
        }

        .btn-on {
            padding: 8px 16px;
            border-radius: 8px;
            font-weight: 600;
            font-size: 0.9rem;
            cursor: pointer;
            transition: all 0.2s ease;
            border: none;
            background-color: var(--primary);
            color: #ffffff;
        }

        .btn-on:hover {
            background-color: var(--primary-hover);
            box-shadow: var(--glow-green);
            transform: translateY(-1px);
        }

        .btn-off {
            padding: 8px 16px;
            border-radius: 8px;
            font-weight: 600;
            font-size: 0.9rem;
            cursor: pointer;
            transition: all 0.2s ease;
            border: none;
            background-color: var(--danger);
            color: #ffffff;
        }

        .btn-off:hover {
            background-color: var(--danger-hover);
            box-shadow: var(--glow-red);
            transform: translateY(-1px);
        }

        .nav-links {
            display: grid;
            grid-template-columns: 1fr 1fr;
            gap: 12px;
            margin-top: 10px;
        }

        .nav-btn {
            display: flex;
            justify-content: center;
            align-items: center;
            padding: 14px;
            background: var(--card-bg);
            border: 1px solid var(--border-color);
            border-radius: 12px;
            color: var(--text-primary);
            text-decoration: none;
            font-weight: 600;
            font-size: 1rem;
            transition: all 0.2s ease;
            box-shadow: var(--shadow);
        }

        .nav-btn:hover {
            border-color: var(--primary);
            background: rgba(16, 185, 129, 0.1);
            color: var(--primary);
            transform: translateY(-2px);
        }

        .control-title {
            margin-bottom: 15px;
            font-weight: 600;
        }

        .msg-feedback {
            text-align: center;
            padding: 10px;
            margin-top: 10px;
            border-radius: 8px;
            font-weight: 600;
            display: none;
        }

        .msg-sucesso {
            background-color: rgba(16, 185, 129, 0.15);
            color: var(--primary);
            border: 1px solid rgba(16, 185, 129, 0.3);
        }

        .msg-erro {
            background-color: rgba(239, 68, 68, 0.15);
            color: var(--danger);
            border: 1px solid rgba(239, 68, 68, 0.3);
        }

        .msg-aviso {
            background-color: rgba(245, 158, 11, 0.15);
            color: #f59e0b;
            border: 1px solid rgba(245, 158, 11, 0.3);
        }

        .nav-back {
            display: flex;
            justify-content: center;
            align-items: center;
            padding: 12px;
            background: transparent;
            border: 1px solid var(--border-color);
            border-radius: 8px;
            color: var(--text-secondary);
            text-decoration: none;
            font-weight: 600;
            font-size: 0.95rem;
            transition: all 0.2s ease;
            text-align: center;
        }

        .nav-back:hover {
            border-color: rgba(255, 255, 255, 0.3);
            color: var(--text-primary);
            background: rgba(255, 255, 255, 0.05);
        }

        footer {
            margin-top: 40px;
            text-align: center;
            color: var(--text-secondary);
            font-size: 0.8rem;
        }
    </style>

    <title>IrrigaSolo - Dashboard</title>
</head>

<body>
    <div class="container">
        <header>
            <h1 id="titulo_dashboard">IrrigaSolo</h1>
            <div class="subtitle" id="subtitulo_dashboard">Painel de Controle de Irrigação</div>
        </header>

        <!-- Relógio do Sistema -->
        <div class="card datetime-display" id="card_relogio">
            <div class="time-large" id="lbl_hora">00:00:00</div>
            <div class="date-sub" id="lbl_data">-- de -- de ----</div>
        </div>

        <!-- Status do Sistema -->
        <div class="card" id="card_status">
            <div class="status-row">
                <span class="status-label">Conexão Wi-Fi</span>
                <span class="status-value" id="status_wifi">
                    <span class="status-badge badge-connected" id="badge_wifi">Conectado</span>
                </span>
            </div>
            <div class="status-row">
                <span class="status-label">IP do Dispositivo</span>
                <span class="status-value" id="val_ip">0.0.0.0</span>
            </div>
            <div class="status-row">
                <span class="status-label">Conexão MQTT</span>
                <span class="status-value" id="status_mqtt">
                    <span class="status-badge badge-disconnected" id="badge_mqtt">Desconectado</span>
                </span>
            </div>
        </div>

        <!-- Controle da Válvula & Medição -->
        <div class="card" id="card_controles">
            <h3 class="control-title">Controle & Medição</h3>

            <!-- Válvula -->
            <div class="channel-control" id="controle_c1">
                <div class="channel-info">
                    <span class="channel-name">Válvula de Irrigação</span>
                    <span class="channel-status" id="lbl_status_c1">Status: <span class="status-off"
                            id="val_status_c1">Desligado</span></span>
                </div>
                <div class="btn-group">
                    <button class="btn-on" id="btn_c1_ligar" onclick="controlarCanal(1, 1)">Ligar</button>
                    <button class="btn-off" id="btn_c1_desligar" onclick="controlarCanal(1, 0)">Desligar</button>
                </div>
            </div>

            <!-- Medições de Fluxo (RF3) -->
            <div class="flow-display" style="margin-top: 15px; padding-top: 15px; border-top: 1px solid #eee;">
                <div class="status-row">
                    <span class="status-label">Vazão Instantânea</span>
                    <span class="status-value" id="val_vazao" style="font-weight: bold;">0.00 L/min</span>
                </div>
                <div class="status-row">
                    <span class="status-label">Volume Acumulado</span>
                    <span class="status-value" id="val_volume" style="font-weight: bold;">0.00 Litros</span>
                </div>
                <div style="text-align: right; margin-top: 10px;">
                    <button class="nav-btn" style="padding: 6px 12px; font-size: 0.85em; background-color: #d9534f;"
                        onclick="zerarVolume()">Zerar Volume</button>
                </div>
            </div>
        </div>

        <!-- Links de Navegação -->
        <div class="nav-links">
            <a href="/horarios" class="nav-btn" id="link_horarios">📅 Agendamentos</a>
            <a href="/config" class="nav-btn" id="link_config">⚙️ Configurações</a>
        </div>

                <footer>IrrigaSolo v1.0 • @copyright Ronaldo Borges - 2026</footer>

    </div>

    <script>
        /**
         * Atualiza os dados dinâmicos do Dashboard via Fetch API.
         * Consome GET /api/status a cada 2 segundos sem recarregar a página.
         */
        function atualizarStatus() {
            fetch('/api/status')
                .then(function (resposta) { return resposta.json(); })
                .then(function (dados) {
                    document.getElementById('lbl_hora').innerText = dados.time || '00:00:00';
                    document.getElementById('lbl_data').innerText = dados.date || '-- de -- de ----';

                    var badgeWifi = document.getElementById('badge_wifi');
                    if (dados.wifiConnected) {
                        badgeWifi.innerText = 'Conectado';
                        badgeWifi.className = 'status-badge badge-connected';
                    } else {
                        badgeWifi.innerText = 'Modo AP';
                        badgeWifi.className = 'status-badge badge-disconnected';
                    }

                    document.getElementById('val_ip').innerText = dados.ip || '0.0.0.0';

                    var badgeMqtt = document.getElementById('badge_mqtt');
                    if (dados.mqttConnected) {
                        badgeMqtt.innerText = 'Conectado';
                        badgeMqtt.className = 'status-badge badge-connected';
                    } else {
                        badgeMqtt.innerText = 'Desconectado';
                        badgeMqtt.className = 'status-badge badge-disconnected';
                    }

                    // Atualizar estado da Válvula (Canal 1)
                    var statusC1 = document.getElementById('val_status_c1');
                    if (dados.c1State) {
                        statusC1.innerText = 'Ligado';
                        statusC1.className = 'status-on';
                    } else {
                        statusC1.innerText = 'Desligado';
                        statusC1.className = 'status-off';
                    }

                    // Atualizar dados do sensor de fluxo
                    document.getElementById('val_vazao').innerText = (dados.vazao !== undefined ? dados.vazao.toFixed(2) : '0.00') + ' L/min';
                    document.getElementById('val_volume').innerText = (dados.volume !== undefined ? dados.volume.toFixed(2) : '0.00') + ' Litros';
                })
                .catch(function (erro) { console.error('Erro ao obter status:', erro); });
        }

        /**
         * Envia comando de controle manual via POST /api/control.
         * Não interfere na programação automática (RF4.1).
         */
        function controlarCanal(canal, estado) {
            fetch('/api/control?channel=' + canal + '&state=' + estado, { method: 'POST' })
                .then(function (resposta) {
                    if (resposta.ok) {
                        atualizarStatus();
                    } else {
                        alert('Erro ao enviar comando.');
                    }
                })
                .catch(function (erro) { console.error('Erro no controle manual:', erro); });
        }

        /**
         * Zera o volume acumulado de água via POST /api/zerar.
         */
        function zerarVolume() {
            if (confirm('Deseja realmente zerar o volume acumulado de água?')) {
                fetch('/api/zerar', { method: 'POST' })
                    .then(function (resposta) {
                        if (resposta.ok) {
                            atualizarStatus();
                        } else {
                            alert('Erro ao zerar volume.');
                        }
                    })
                    .catch(function (erro) { console.error('Erro ao zerar volume:', erro); });
            }
        }

        // Polling a cada 2 segundos para atualização em tempo real
        setInterval(atualizarStatus, 2000);
        // Atualização inicial ao carregar a página
        window.onload = atualizarStatus;
    </script>
</body>

</html>
)rawliteral";
