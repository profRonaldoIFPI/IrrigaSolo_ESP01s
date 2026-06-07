#pragma once

const char config_html[] PROGMEM = R"rawliteral(
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

    <title>IrrigaSolo - Configurações</title>
    <style>
        .form-group {
            margin-bottom: 16px;
        }

        .form-group label {
            display: block;
            color: var(--text-secondary);
            font-weight: 500;
            font-size: 0.9rem;
            margin-bottom: 6px;
        }

        .form-group input[type="text"],
        .form-group input[type="password"],
        .form-group input[type="number"] {
            width: 100%;
            padding: 10px 12px;
            border-radius: 8px;
            border: 1px solid var(--border-color);
            background: rgba(15, 23, 42, 0.6);
            color: var(--text-primary);
            font-size: 0.95rem;
            outline: none;
            transition: border-color 0.2s ease;
        }

        .form-group input:focus {
            border-color: var(--primary);
        }

        .form-hint {
            color: var(--text-secondary);
            font-size: 0.8rem;
            margin-top: 4px;
        }

        /* Estilização premium do input de arquivo de upload OTA */
        .file-input {
            width: 100%;
            padding: 10px 12px;
            border-radius: 8px;
            border: 1px solid var(--border-color);
            background: rgba(15, 23, 42, 0.6);
            color: var(--text-primary);
            font-size: 0.95rem;
            outline: none;
            cursor: pointer;
        }

        .file-input::file-selector-button {
            background-color: var(--primary);
            color: #ffffff;
            border: none;
            padding: 6px 12px;
            border-radius: 6px;
            font-weight: 600;
            font-size: 0.85rem;
            cursor: pointer;
            margin-right: 12px;
            transition: background-color 0.2s ease;
        }

        .file-input::file-selector-button:hover {
            background-color: var(--primary-hover);
        }

        .btn-danger-ota {
            background-color: var(--danger) !important;
            margin-top: 5px;
        }

        .btn-danger-ota:hover {
            background-color: var(--danger-hover) !important;
            box-shadow: var(--glow-red) !important;
        }
    </style>
</head>

<body>
    <div class="container">
        <header>
            <h1 id="titulo_config">Configurações</h1>
            <div class="subtitle" id="subtitulo_config">Rede Wi-Fi e Sincronização de Tempo</div>
        </header>

        <!-- Configuração da Rede Local (STA) -->
        <div class="card" id="card_config_sta">
            <h2>Rede Local (Wi-Fi)</h2>
            <div class="form-group">
                <label for="campo_ssid_sta">SSID da Rede</label>
                <input type="text" id="campo_ssid_sta" placeholder="Nome da sua rede Wi-Fi" maxlength="32">
            </div>
            <div class="form-group">
                <label for="campo_senha_sta">Senha da Rede</label>
                <input type="password" id="campo_senha_sta" placeholder="Senha da sua rede Wi-Fi" maxlength="63">
                <div class="form-hint">Deixe em branco para manter a senha atual.</div>
            </div>
        </div>

        <!-- Configuração do Access Point -->
        <div class="card" id="card_config_ap">
            <h2>Access Point (AP)</h2>
            <div class="form-group">
                <label for="campo_ssid_ap">SSID do AP</label>
                <input type="text" id="campo_ssid_ap" placeholder="IrrigaSolo" maxlength="32">
            </div>
            <div class="form-group">
                <label for="campo_senha_ap">Senha do AP</label>
                <input type="password" id="campo_senha_ap" placeholder="Senha do Access Point" maxlength="63">
                <div class="form-hint">Deixe em branco para manter a senha atual.</div>
            </div>
        </div>

        <!-- Configuração NTP -->
        <div class="card" id="card_config_ntp">
            <h2>Sincronização de Tempo (NTP)</h2>
            <div class="form-group">
                <label for="campo_servidor_ntp">Servidor NTP</label>
                <input type="text" id="campo_servidor_ntp" placeholder="pool.ntp.br" maxlength="64">
            </div>
            <div class="form-group">
                <label for="campo_fuso_horario">Fuso Horário (UTC)</label>
                <input type="number" id="campo_fuso_horario" value="-3" min="-12" max="14">
                <div class="form-hint">Ex: -3 para Brasília, 0 para GMT.</div>
            </div>
            <div class="form-group">
                <label for="campo_intervalo_ntp">Intervalo de Sincronização (segundos)</label>
                <input type="number" id="campo_intervalo_ntp" value="21600" min="60" max="86400">
                <div class="form-hint">Padrão: 21600 (6 horas). Mínimo: 60s.</div>
            </div>
        </div>

        <!-- Atualização de Firmware (OTA) -->
        <div class="card" id="card_config_ota">
            <h2>Atualização de Firmware (OTA)</h2>
            <form method="POST" action="/update" enctype="multipart/form-data" id="form_ota">
                <div class="form-group">
                    <label for="arquivo_ota">Selecione o arquivo firmware.bin</label>
                    <input type="file" id="arquivo_ota" name="update" accept=".bin" class="file-input" required>
                </div>
                <button type="submit" class="btn-submit btn-danger-ota" id="btn_update_ota">
                    Realizar Atualização
                </button>
            </form>
            <div class="form-hint" style="margin-top: 8px;">Atenção: O dispositivo irá reiniciar após o término do upload.</div>
        </div>

        <!-- Mensagem de feedback -->
        <div class="msg-feedback" id="msg_feedback"></div>

        <button class="btn-submit" id="btn_salvar_config" onclick="salvarConfiguracaoRede()"
            style="margin-bottom: 20px;">
            Salvar Configurações
        </button>

        <a href="/" class="nav-back" id="link_voltar_painel">⬅ Voltar ao Painel</a>

                <footer>IrrigaSolo v1.0 • @copyright Ronaldo Borges - 2026</footer>

    </div>

    <script>
        /**
         * Carrega a configuração de rede atual via GET /api/network.
         * Senhas não são retornadas pelo backend (segurança).
         */
        function carregarConfiguracaoRede() {
            fetch('/api/network')
                .then(function (resposta) { return resposta.json(); })
                .then(function (dados) {
                    document.getElementById('campo_ssid_ap').value = dados.ssidAp || '';
                    document.getElementById('campo_ssid_sta').value = dados.ssidSta || '';
                    document.getElementById('campo_servidor_ntp').value = dados.ntpServer || 'pool.ntp.br';
                    document.getElementById('campo_fuso_horario').value = dados.timeZone !== undefined ? dados.timeZone : -3;
                    document.getElementById('campo_intervalo_ntp').value = dados.ntpSyncInterval || 21600;
                })
                .catch(function (erro) { console.error('Erro ao carregar config de rede:', erro); });
        }

        /**
         * Envia a nova configuração de rede via POST /api/network (JSON).
         * Campos de senha em branco são omitidos para manter os valores atuais.
         */
        function salvarConfiguracaoRede() {
            var botao = document.getElementById('btn_salvar_config');
            botao.disabled = true;
            botao.innerText = 'Salvando...';

            var payload = {
                ssidAp: document.getElementById('campo_ssid_ap').value,
                ssidSta: document.getElementById('campo_ssid_sta').value,
                ntpServer: document.getElementById('campo_servidor_ntp').value,
                timeZone: parseInt(document.getElementById('campo_fuso_horario').value) || -3,
                ntpSyncInterval: parseInt(document.getElementById('campo_intervalo_ntp').value) || 21600
            };

            // Incluir senhas apenas se preenchidas
            var senhaAp = document.getElementById('campo_senha_ap').value;
            if (senhaAp.length > 0) { payload.pwdAp = senhaAp; }

            var senhaSta = document.getElementById('campo_senha_sta').value;
            if (senhaSta.length > 0) { payload.pwdSta = senhaSta; }

            fetch('/api/network', {
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
                        msgEl.className = 'msg-feedback msg-aviso';
                        msgEl.innerText = '⚠️ Configuração salva. O dispositivo está reconectando à rede...';
                    } else {
                        msgEl.className = 'msg-feedback msg-erro';
                        msgEl.innerText = '❌ ' + (resultado.dados.error || 'Erro desconhecido.');
                    }

                    setTimeout(function () { msgEl.style.display = 'none'; }, 8000);
                })
                .catch(function (erro) {
                    var msgEl = document.getElementById('msg_feedback');
                    msgEl.style.display = 'block';
                    msgEl.className = 'msg-feedback msg-erro';
                    msgEl.innerText = '❌ Erro de comunicação com o dispositivo.';
                    console.error('Erro ao salvar config:', erro);
                })
                .finally(function () {
                    botao.disabled = false;
                    botao.innerText = 'Salvar Configurações';
                });
        }

        window.onload = function() {
            carregarConfiguracaoRede();
            
            // Adiciona feedback visual ao envio do firmware
            var formOta = document.getElementById('form_ota');
            if (formOta) {
                formOta.onsubmit = function() {
                    var btn = document.getElementById('btn_update_ota');
                    btn.disabled = true;
                    btn.innerText = 'Enviando firmware...';
                    btn.style.backgroundColor = '#475569';
                };
            }
        };
    </script>
</body>

</html>
)rawliteral";
