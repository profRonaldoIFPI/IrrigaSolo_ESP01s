#pragma once

const char horarios_html[] PROGMEM = R"rawliteral(
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

    <title>IrrigaSolo - Programação</title>
    <style>
        .table-responsive {
            width: 100%;
            overflow-x: auto;
            margin-bottom: 20px;
        }

        table {
            width: 100%;
            border-collapse: collapse;
            text-align: left;
        }

        th {
            padding: 10px;
            font-weight: 600;
            color: var(--text-secondary);
            border-bottom: 1px solid rgba(255, 255, 255, 0.1);
            font-size: 0.9rem;
        }

        td {
            padding: 8px 10px;
            border-bottom: 1px solid rgba(255, 255, 255, 0.05);
            vertical-align: middle;
        }

        input[type="time"] {
            padding: 6px 10px;
            border-radius: 6px;
            border: 1px solid var(--border-color);
            background: rgba(15, 23, 42, 0.6);
            color: var(--text-primary);
            font-size: 0.95rem;
            outline: none;
            width: 100%;
            max-width: 130px;
        }

        input[type="number"] {
            padding: 6px 10px;
            border-radius: 6px;
            border: 1px solid var(--border-color);
            background: rgba(15, 23, 42, 0.6);
            color: var(--text-primary);
            font-size: 0.95rem;
            outline: none;
            width: 100%;
            max-width: 80px;
        }

        input[type="checkbox"] {
            width: 18px;
            height: 18px;
            accent-color: var(--primary);
            cursor: pointer;
        }

        .option-group {
            display: flex;
            align-items: center;
            gap: 20px;
            margin-top: 10px;
            padding: 12px;
            background: rgba(255, 255, 255, 0.02);
            border-radius: 8px;
            border: 1px solid rgba(255, 255, 255, 0.05);
        }

        .option-label {
            font-weight: 600;
            font-size: 0.95rem;
            color: var(--text-primary);
        }

        .radio-item {
            display: flex;
            align-items: center;
            gap: 6px;
            cursor: pointer;
        }

        .radio-item input[type="radio"] {
            accent-color: var(--primary);
            width: 16px;
            height: 16px;
            cursor: pointer;
        }

        #btn_salvar_horarios {
            margin-bottom: 20px;
        }
    </style>
</head>

<body>
    <div class="container">
        <header>
            <h1 id="titulo_horarios">Programação de Irrigação</h1>
            <div class="subtitle" id="subtitulo_horarios">Defina os horários e durações de cada canal</div>
        </header>

        <!-- Canal 1 -->
        <div class="card" id="card_c1_agendamentos">
            <h2>Válvula de Irrigação (Canal 1)</h2>
            <div class="table-responsive">
                <table id="tbl_c1">
                    <thead>
                        <tr>
                            <th style="width: 15%;">Ativo</th>
                            <th style="width: 45%;">Horário de Início</th>
                            <th style="width: 40%;">Duração (min)</th>
                        </tr>
                    </thead>
                    <tbody id="tbody_c1"></tbody>
                </table>
            </div>
        </div>

        <!-- Mensagem de feedback -->
        <div class="msg-feedback" id="msg_feedback"></div>

        <button class="btn-submit" id="btn_salvar_horarios" onclick="salvarAgendamentos()">
            Salvar Programação
        </button>

        <a href="/" class="nav-back" id="link_voltar_painel">⬅ Voltar ao Painel</a>

                <footer>IrrigaSolo v1.0 • @copyright Ronaldo Borges - 2026</footer>

    </div>

    <script>
        var TOTAL_SLOTS = 6;

        /**
         * Gera dinamicamente as linhas da tabela de horários.
         * Evita duplicação de HTML estático.
         */
        function gerarLinhasTabela() {
            var tbodyC1 = document.getElementById('tbody_c1');

            for (var i = 0; i < TOTAL_SLOTS; i++) {
                tbodyC1.innerHTML += criarLinhaSlot('c1', i);
            }
        }

        function criarLinhaSlot(prefixo, indice) {
            return '<tr>' +
                '<td><input type="checkbox" id="' + prefixo + '_ativo_' + indice + '"></td>' +
                '<td><input type="time" id="' + prefixo + '_inicio_' + indice + '" value="00:00"></td>' +
                '<td><input type="number" id="' + prefixo + '_duracao_' + indice + '" value="10" min="1" max="120"></td>' +
                '</tr>';
        }

        /**
         * Carrega as configurações atuais do backend via GET /api/schedules.
         */
        function carregarAgendamentos() {
            fetch('/api/schedules')
                .then(function (resposta) { return resposta.json(); })
                .then(function (dados) {
                    // Canal 1
                    if (dados.channel1) {
                        dados.channel1.forEach(function (sessao, idx) {
                            if (idx < TOTAL_SLOTS) {
                                document.getElementById('c1_ativo_' + idx).checked = sessao.enabled;
                                document.getElementById('c1_inicio_' + idx).value = sessao.start || '00:00';
                                document.getElementById('c1_duracao_' + idx).value = sessao.duration || 10;
                            }
                        });
                    }
                })
                .catch(function (erro) { console.error('Erro ao carregar agendamentos:', erro); });
        }

        /**
         * Envia os agendamentos ao backend via POST /api/schedules (JSON).
         */
        function salvarAgendamentos() {
            var botao = document.getElementById('btn_salvar_horarios');
            botao.disabled = true;
            botao.innerText = 'Salvando...';

            var payload = {
                channel1: []
            };

            for (var i = 0; i < TOTAL_SLOTS; i++) {
                payload.channel1.push({
                    enabled: document.getElementById('c1_ativo_' + i).checked,
                    start: document.getElementById('c1_inicio_' + i).value || '00:00',
                    duration: parseInt(document.getElementById('c1_duracao_' + i).value) || 0
                });
            }

            fetch('/api/schedules', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify(payload)
            })
                .then(function (resposta) {
                    return resposta.json().then(function (dados) {
                        return { ok: resposta.ok, dados: dados };
                    });
                })
                .then(function (resultado) {
                    var msgEl = document.getElementById('msg_feedback');
                    msgEl.style.display = 'block';

                    if (resultado.ok) {
                        msgEl.className = 'msg-feedback msg-sucesso';
                        msgEl.innerText = '✅ Programação salva com sucesso!';
                    } else {
                        msgEl.className = 'msg-feedback msg-erro';
                        msgEl.innerText = '❌ ' + (resultado.dados.error || 'Erro desconhecido.');
                    }

                    // Esconder mensagem após 5 segundos
                    setTimeout(function () { msgEl.style.display = 'none'; }, 5000);
                })
                .catch(function (erro) {
                    var msgEl = document.getElementById('msg_feedback');
                    msgEl.style.display = 'block';
                    msgEl.className = 'msg-feedback msg-erro';
                    msgEl.innerText = '❌ Erro de comunicação com o dispositivo.';
                    console.error('Erro ao salvar:', erro);
                })
                .finally(function () {
                    botao.disabled = false;
                    botao.innerText = 'Salvar Programação';
                });
        }

        // Inicialização
        gerarLinhasTabela();
        window.onload = carregarAgendamentos;
    </script>
</body>

</html>
)rawliteral";
