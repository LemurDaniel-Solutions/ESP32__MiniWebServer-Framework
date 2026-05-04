// ESP32 MiniWebServer Framework - https://github.com/LemurDaniel/ESP32__MiniWebServer-Framework
// Copyright © 2026, Daniel Landau
// MIT License

#include <utils/utility.admin.h>

namespace EspWeb
{

    static void restartTask(void *param)
    {
        vTaskDelay(pdMS_TO_TICKS(500));
        ESP.restart();
        vTaskDelete(nullptr);
    }

    /*-------------------------------------------------------------------------------------------------
     *
     * Admin Pages
     *
     **/

    void get_AdminLogin(Request &req, Response &res)
    {
        std::string adminPage = R"html(
<!DOCTYPE html>
<html lang="de">

<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Admin Login</title>
    <style>
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background-color: #f4f7f6;
            display: flex;
            justify-content: center;
            align-items: center;
            height: 100vh;
            margin: 0;
        }

        .login-container {
            background: white;
            padding: 2rem;
            border-radius: 8px;
            box-shadow: 0 4px 6px rgba(0, 0, 0, 0.1);
            width: 100%;
            max-width: 400px;
        }

        h2 {
            text-align: center;
            color: #333;
            margin-bottom: 1.5rem;
        }

        .form-group {
            margin-bottom: 1rem;
        }

        label {
            display: block;
            margin-bottom: 0.5rem;
            color: #666;
        }

        input {
            width: 100%;
            padding: 0.75rem;
            border: 1px solid #ddd;
            border-radius: 4px;
            box-sizing: border-box;
        }

        button {
            width: 100%;
            padding: 0.75rem;
            background-color: #007bff;
            border: none;
            border-radius: 4px;
            color: white;
            font-size: 1rem;
            cursor: pointer;
            transition: background 0.3s;
        }

        button:hover {
            background-color: #0056b3;
        }
    </style>
</head>

<body>

    <div class="login-container">
        <h2>Admin Only</h2>
        <form id="form-login">
            <div class="form-group">
                <label for="username">Benutzername</label>
                <input type="text" id="username" name="username" required>
            </div>
            <div class="form-group">
                <label for="password">Passwort</label>
                <input type="password" id="password" name="password" required>
            </div>
            <button type="submit">Einloggen</button>
        </form>
    </div>

    <script>
        window.onload = function () {
            document.getElementById("form-login").onsubmit = function () {
                postLogin();
                return false;
            };
        };

        async function postLogin() {
            const form = document.getElementById('form-login');

            try {
                const res = await fetch('admin/login', {
                    method: 'POST',
                    headers: {
                        'Content-Type': 'application/json'
                    },
                    body: JSON.stringify({
                        username: form.username.value,
                        password: form.password.value
                    })
                });

                if(!res.ok) {
                    alert('Login fehlgeschlagen: ' + res.statusText);
                    return;
                }

                const json = await res.json();
                const token = json.token;

                document.cookie = `adminToken=${token}; path=/; max-age=3600`;

                window.location.replace('/admin/dashboard')
            }
            catch (error) {
                console.error('Fehler beim Login:', error);
            }

        }

    </script>

</body>
</html>
)html";

        res.html(adminPage);
    }

    void get_AdminDashboard(Request &req, Response &res)
    {
        std::string dashboardPage = R"html(
<!DOCTYPE html>
<html lang="en">

<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ESP32 Admin Panel</title>

    <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.4.0/css/all.min.css">

    <style>
        :root {
            --bg-color: #f0f2f5;
            --card-bg: #ffffff;
            --text-main: #2c3e50;
            --primary: #3498db;
            --danger: #e74c3c;
            --border-radius: 12px;
        }

        body {
            font-family: 'Segoe UI', Tahoma, sans-serif;
            background-color: var(--bg-color);
            color: var(--text-main);
            margin: 0;
            padding: 20px;
        }

        .container {
            max-width: 1000px;
            margin: 0 auto;
        }

        header {
            display: flex;
            justify-content: space-between;
            align-items: baseline;
            margin-bottom: 30px;
            padding-bottom: 15px;
            border-bottom: 2px solid #ddd;
        }

        .header-left {
            display: flex;
            align-items: center;
            gap: 15px;
        }

        .grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(320px, 1fr));
            gap: 25px;
        }

        .card {
            background: var(--card-bg);
            padding: 25px;
            border-radius: var(--border-radius);
            box-shadow: 0 4px 20px rgba(0, 0, 0, 0.08);
            position: relative;
        }

        .card h2 {
            margin-top: 0;
            font-size: 1rem;
            color: #95a5a6;
            text-transform: uppercase;
            letter-spacing: 1.2px;
            display: flex;
            align-items: center;
            gap: 10px;
        }

        .card h2 i {
            color: var(--primary);
            font-size: 1.2rem;
        }

        .info-group {
            margin: 20px 0;
        }

        .label {
            display: block;
            font-size: 0.8rem;
            color: #bdc3c7;
            text-transform: uppercase;
            font-weight: bold;
        }

        .value {
            font-size: 1.3rem;
            font-weight: 600;
            color: var(--text-main);
            display: flex;
            align-items: center;
            gap: 10px;
        }

        .btn {
            display: inline-flex;
            align-items: center;
            gap: 8px;
            padding: 12px 20px;
            background: var(--primary);
            color: white;
            text-decoration: none;
            border-radius: 6px;
            font-weight: 600;
            border: none;
            cursor: pointer;
            transition: transform 0.1s, background 0.2s;
        }

        .btn:hover {
            background: #2980b9;
            transform: translateY(-1px);
        }

        .btn-danger {
            background: var(--danger);
        }

        .btn-danger:hover {
            background: #c0392b;
        }

        .form-group {
            margin-top: 15px;
        }

        input {
            width: 100%;
            padding: 10px;
            margin: 8px 0 15px 0;
            border: 1px solid #dcdde1;
            border-radius: 6px;
            box-sizing: border-box;
            background: #f9f9f9;
        }

        .website-upload {
            color: white !important;
            border: 2px solid;
            box-sizing: border-box;
            background: var(--primary);
            border-radius: 6px;
            padding: 10px 5px 0px 5px;
            margin: 8px 0px 15px 0;
            display: inline-block;
            cursor: pointer;
            transition: transform 0.1s, background 0.2s;
        }

        .website-upload h3 {
            color: white !important;
        }

        progress {
            border-radius: 7px;
            height: 14px;
            box-shadow: 1px 1px 4px rgba(0, 0, 0, 0.2);
        }

        progress::-webkit-progress-bar {
            background: white;
            border-radius: 7px;
        }

        progress::-webkit-progress-value {
            background: var(--primary);
            border-radius: 7px;
        }

        .website-upload:hover {
            background: #2980b9;
            transform: translateY(-1px);
        }

        input[type="file"] {
            display: none;
        }

        input:focus {
            outline: none;
            border-color: var(--primary);
            background: #fff;
        }

        select {
            width: 100%;
            padding: 10px;
            margin: 8px 0 15px 0;
            border: 1px solid #dcdde1;
            border-radius: 6px;
            box-sizing: border-box;
            background: #f9f9f9;
            font-size: 1rem;
            color: var(--text-main);
            cursor: pointer;
        }

        select:focus {
            outline: none;
            border-color: var(--primary);
            background: #fff;
        }

        .network-list {
            margin-top: 15px;
            display: flex;
            flex-direction: column;
            gap: 10px;
        }

        .network-item {
            display: flex;
            align-items: center;
            gap: 10px;
            padding: 10px 12px;
            background: #f4f6f8;
            border-radius: 8px;
            border: 1px solid #e0e0e0;
        }

        .network-item .net-info {
            flex: 1;
            min-width: 0;
        }

        .network-item .net-ssid {
            font-weight: 600;
            font-size: 0.95rem;
            white-space: nowrap;
            overflow: hidden;
            text-overflow: ellipsis;
        }

        .network-item .btn-icon {
            background: none;
            border: none;
            cursor: pointer;
            color: #bdc3c7;
            font-size: 1rem;
            padding: 4px 6px;
            border-radius: 4px;
            transition: color 0.2s;
            flex-shrink: 0;
        }

        .network-item .btn-icon:hover {
            color: var(--danger);
        }

        .add-network-form {
            margin-top: 15px;
            border-top: 1px solid #eee;
            padding-top: 15px;
            display: none;
        }

        .btn-row {
            display: flex;
            gap: 10px;
            margin-top: 5px;
        }

        .btn-sm {
            padding: 8px 14px;
            font-size: 0.85rem;
        }

        .btn-secondary {
            background: #7f8c8d;
        }

        .btn-secondary:hover {
            background: #636e72;
        }

        .btn-success {
            background: #27ae60;
        }

        .btn-success:hover {
            background: #219a52;
        }

        .modal-overlay {
            display: none;
            position: fixed;
            inset: 0;
            background: rgba(0, 0, 0, 0.45);
            z-index: 1000;
            align-items: center;
            justify-content: center;
        }

        .modal-overlay.visible {
            display: flex;
        }

        .modal {
            background: var(--card-bg);
            border-radius: var(--border-radius);
            padding: 35px 30px 25px;
            max-width: 420px;
            width: 90%;
            box-shadow: 0 12px 40px rgba(0, 0, 0, 0.2);
            text-align: center;
            animation: pop-in 0.2s ease;
        }

        @keyframes pop-in {
            from {
                transform: scale(0.85);
                opacity: 0;
            }

            to {
                transform: scale(1);
                opacity: 1;
            }
        }

        .modal-icon {
            font-size: 2.8rem;
            color: var(--primary);
            margin-bottom: 15px;
        }

        .modal h3 {
            margin: 0 0 10px;
            font-size: 1.25rem;
            color: var(--text-main);
        }

        .modal p {
            color: #7f8c8d;
            font-size: 0.95rem;
            margin: 0 0 25px;
        }

        .modal .btn-row {
            justify-content: center;
        }

        .add-token-form {
            margin-top: 15px;
            border-top: 1px solid #eee;
            padding-top: 15px;
            display: none;
        }

        .token-value {
            background: #f4f6f8;
            border-radius: 6px;
            padding: 12px;
            font-family: monospace;
            font-size: 0.82rem;
            word-break: break-all;
            text-align: left;
            margin-bottom: 20px;
            border: 1px solid #e0e0e0;
            user-select: all;
        }
    </style>
</head>

<body>
    <div class="container">
        <header>
            <div class="header-left">
                <h1>Admin Dashboard</h1>
            </div>
            <div style="display:flex; gap:10px;">
                <button onclick="openModal('modal-upload-website')" class="btn"><i class="fa-solid fa-folder"></i>
                    Upload Website</button>
                <button onclick="confirmRestart()" class="btn btn-success"><i class="fa-solid fa-power-off"></i> Restart
                    Device</button>
                <a href="/admin" onClick="logOut()" class="btn btn-danger"><i
                        class="fa-solid fa-right-from-bracket"></i> Logout</a>
            </div>
        </header>

        <div class="grid">
            <div class="card">
                <h2><i class="fa-solid fa-wifi"></i> Active Connection</h2>
                <div class="info-group">
                    <span class="label">SSID</span>
                    <div class="value" id="ssid-value">---</div>
                </div>
                <div class="info-group">
                    <span class="label">Password</span>
                    <div class="value" id="password-value">---</div>
                </div>
                <div class="info-group">
                    <span class="label">Signal Strength</span>
                    <div class="value"><i class="fa-solid fa-signal"></i> <span id="rssi-value">0</span></div>
                </div>
                <div class="info-group">
                    <span class="label">IP Address</span>
                    <div class="value" style="font-family: monospace; font-size: 1.1rem;" id="ip-value">0.0.0.0</div>
                </div>
            </div>

            <div class="card">
                <h2><i class="fa-solid fa-list"></i> Saved Networks</h2>
                <div class="network-list" id="network-list">
                    <div style="color:#bdc3c7; font-size:0.9rem;">Loading...</div>
                </div>
                <div class="add-network-form" id="add-network-form">
                    <div class="form-group">
                        <label class="label">Network</label>
                        <select id="wifi-ssid">
                            <option value="">-- Scanning... --</option>
                        </select>
                    </div>
                    <div class="form-group">
                        <label class="label">Password</label>
                        <input type="password" id="wifi-password" placeholder="WiFi Password">
                    </div>
                    <div class="btn-row">
                        <button onclick="addNetwork()" class="btn btn-sm"><i class="fa-solid fa-floppy-disk"></i>
                            Save</button>
                        <button onclick="scanWiFi()" class="btn btn-sm btn-secondary"><i class="fa-solid fa-rotate"></i>
                            Rescan</button>
                        <button onclick="toggleAddForm(false)" class="btn btn-sm btn-secondary"><i
                                class="fa-solid fa-xmark"></i> Cancel</button>
                    </div>
                </div>
                <div style="margin-top:15px;">
                    <button onclick="toggleAddForm(true)" class="btn btn-sm" id="btn-add-network">
                        <i class="fa-solid fa-plus"></i> Add Network
                    </button>
                </div>
            </div>

            <div class="card">
                <h2><i class="fa-solid fa-user-shield"></i> Security</h2>
                <form id="form-creds">
                    <div class="form-group">
                        <label class="label">Admin Username</label>
                        <input type="text" name="admin_user" placeholder="Username" required>
                    </div>
                    <div class="form-group">
                        <label class="label">New Password</label>
                        <input type="password" name="admin_pwd" placeholder="Leave empty to keep current">
                    </div>
                    <button type="submit" class="btn"><i class="fa-solid fa-floppy-disk"></i> Update Auth</button>
                </form>
            </div>

            <div class="card">
                <h2><i class="fa-solid fa-key"></i> API Tokens</h2>
                <div class="network-list" id="token-list">
                    <div style="color:#bdc3c7; font-size:0.9rem;">Loading...</div>
                </div>
                <div class="add-token-form" id="add-token-form">
                    <div class="form-group">
                        <label class="label">Token Name</label>
                        <input type="text" id="token-name" placeholder="z.B. Home Server">
                    </div>
                    <div class="btn-row">
                        <button onclick="createToken()" class="btn btn-sm"><i class="fa-solid fa-plus"></i>
                            Erstellen</button>
                        <button onclick="toggleTokenForm(false)" class="btn btn-sm btn-secondary"><i
                                class="fa-solid fa-xmark"></i> Abbrechen</button>
                    </div>
                </div>
                <div style="margin-top:15px;">
                    <button onclick="toggleTokenForm(true)" class="btn btn-sm" id="btn-add-token">
                        <i class="fa-solid fa-plus"></i> Neuer Token
                    </button>
                </div>
            </div>
        </div>
    </div>

    <div class="modal-overlay" id="modal-new-token">
        <div class="modal">
            <div class="modal-icon"><i class="fa-solid fa-key"></i></div>
            <h3>Token erstellt</h3>
            <p style="margin-bottom:10px;"><strong id="new-token-name"></strong></p>
            <div class="token-value" id="new-token-value"></div>
            <div class="btn-row">
                <button onclick="copyNewToken()" class="btn btn-sm"><i class="fa-solid fa-copy"></i> Kopieren</button>
                <button onclick="closeModal('modal-new-token')" class="btn btn-sm btn-secondary">Schliessen</button>
            </div>
        </div>
    </div>

    <div class="modal-overlay" id="modal-wifi-saved">
        <div class="modal">
            <div class="modal-icon"><i class="fa-solid fa-circle-check"></i></div>
            <h3>Network saved</h3>
            <p>The network was added. Changes take effect after a restart.</p>
            <div class="btn-row">
                <button onclick="closeModal('modal-wifi-saved')" class="btn btn-secondary btn-sm">Later</button>
                <button onclick="doRestart()" class="btn btn-sm"><i class="fa-solid fa-power-off"></i> Restart
                    Now</button>
            </div>
        </div>
    </div>

    <div class="modal-overlay" id="modal-restart">
        <div class="modal">
            <div class="modal-icon" style="color:var(--danger)"><i class="fa-solid fa-triangle-exclamation"></i></div>
            <h3>Restart device?</h3>
            <p>The ESP32 will be restarted. The connection will be interrupted.</p>
            <div class="btn-row">
                <button onclick="closeModal('modal-restart')" class="btn btn-secondary btn-sm">Cancel</button>
                <button onclick="doRestart()" class="btn btn-danger btn-sm"><i class="fa-solid fa-power-off"></i>
                    Restart</button>
            </div>
        </div>
    </div>

    <div class="modal-overlay" id="modal-admin-saved">
        <div class="modal">
            <div class="modal-icon" style="color:var(--danger)"><i class="fa-solid fa-triangle-exclamation"></i></div>
            <h3>Admin Credentials were update!</h3>
            <p>The old Credentials are no longer valid.</p>
            <div class="btn-row">
                <button onclick="closeModal('modal-admin-saved')" class="btn btn-danger btn-sm">Confirm</button>
            </div>
        </div>
    </div>

    <div class="modal-overlay" id="modal-upload-website">
        <div class="modal">
            <div class="modal-icon"><i class="fa-solid fa-folder"></i></div>
            <label class="website-upload">
                <input id="upload-website-picker" type="file" webkitdirectory directory multiple />
                <h3 id="display-upload-folder">Select Folder to upload</h3>
            </label>
            <progress id="upload-website-progress" max="100" hidden></progress>
            <p style="margin-bottom:10px;"><strong id="new-token-name"></strong></p>
            <div class="btn-row">
                <button id="btn-upload-webiste-close" onclick="closeModal('modal-upload-website')"
                    class="btn btn-sm btn-secondary">Close</button>
            </div>
        </div>
    </div>

    <script>
        function openModal(id) { document.getElementById(id).classList.add('visible'); }
        function closeModal(id) { document.getElementById(id).classList.remove('visible'); }
        function confirmRestart() { openModal('modal-restart'); }

        function escapeHtml(str) {
            return str.replace(/&/g, '&amp;').replace(/</g, '&lt;').replace(/>/g, '&gt;').replace(/"/g, '&quot;');
        }

        function toggleAddForm(open) {
            const form = document.getElementById('add-network-form');
            const btn = document.getElementById('btn-add-network');
            form.style.display = open ? 'block' : 'none';
            btn.style.display = open ? 'none' : 'inline-flex';
            if (open) scanWiFi();
        }

        async function doRestart() {
            closeModal('modal-wifi-saved');
            closeModal('modal-restart');
            await fetch('/admin/restart', { method: 'POST' }).catch(() => { });
        }

        async function loadWiFiConfig() {
            try {
                const res = await fetch('/admin/wifi/active');
                const json = await res.json();
                document.getElementById('ssid-value').innerText = json.SSID || "N/A";
                document.getElementById('password-value').innerText = json.Password || "Not Set";
                document.getElementById('ip-value').innerText = json.IPAddress || "";
                document.getElementById('rssi-value').innerText = json.SignalStrength || "0 dBm";
            } catch (e) { console.error("Fetch error", e); }
        }

        var scanWiFiRunning = false;
        async function scanWiFi() {
            if (scanWiFiRunning) return;
            scanWiFiRunning = true;
            const select = document.getElementById('wifi-ssid');
            select.innerHTML = '<option value="">-- Scanning... --</option>';
            try {
                const res = await fetch('/admin/wifi/scan');
                const json = await res.json();
                select.innerHTML = json.networks
                    .map(e => `<option value="${escapeHtml(e.SSID)}">(${escapeHtml(String(e.SignalStrength))}) ${escapeHtml(e.SSID)}</option>`)
                    .join('');
            } catch (e) {
                console.error("Scan error", e);
                select.innerHTML = '<option value="">-- Scan failed --</option>';
            } finally { scanWiFiRunning = false; }
        }

        async function addNetwork() {
            const ssid = document.getElementById('wifi-ssid').value;
            const password = document.getElementById('wifi-password').value;
            if (!ssid) return;
            await fetch('/admin/wifi/network', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ ssid, password })
            });
            document.getElementById('wifi-password').value = '';
            toggleAddForm(false);
            loadNetworks();
            openModal('modal-wifi-saved');
        }

        async function removeNetwork(ssid) {
            if (!confirm(`Remove "${ssid}" from saved networks?`)) return;
            await fetch('/admin/wifi/network', {
                method: 'DELETE',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ ssid })
            }).catch(() => { });
            loadNetworks();
        }

        function renderNetworkList(networks) {
            const list = document.getElementById('network-list');
            if (!networks || networks.length === 0) {
                list.innerHTML = '<div style="color:#bdc3c7; font-size:0.9rem;">No saved networks.</div>';
                return;
            }
            list.innerHTML = networks.map((net, i) => `
                <div class="network-item" id="net-item-${i}">
                    <div class="net-info">
                        <div class="net-ssid">${escapeHtml(net.SSID)}</div>
                    </div>
                    <button class="btn-icon" title="Remove network" onclick="removeNetwork('${net.SSID}')">
                        <i class="fa-solid fa-trash"></i>
                    </button>
                </div>
            `).join('');
        }

        async function loadNetworks() {
            try {
                const res = await fetch('/admin/wifi/networks');
                const json = await res.json();
                renderNetworkList(json.networks || []);
            } catch (e) {
                console.error("Networks fetch error", e);
                document.getElementById('network-list').innerHTML =
                    '<div style="color:#e74c3c; font-size:0.9rem;">Failed to load networks.</div>';
            }
        }

        async function postAdminCredentials() {
            const form = document.getElementById('form-creds');
            const res = await fetch('/admin/auth', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ admin_user: form.admin_user.value, admin_pwd: form.admin_pwd.value })
            });
            if (!res.ok) { alert(res.statusText); return; }
            openModal('modal-admin-saved');
        }

        async function logOut() { await fetch('/admin/logout'); }

        function toggleTokenForm(open) {
            document.getElementById('add-token-form').style.display = open ? 'block' : 'none';
            document.getElementById('btn-add-token').style.display = open ? 'none' : 'inline-flex';
            if (open) document.getElementById('token-name').value = '';
        }

        async function loadPermTokens() {
            try {
                const res = await fetch('/admin/tokens');
                const json = await res.json();
                const list = document.getElementById('token-list');
                const tokens = json.tokens || [];
                if (tokens.length === 0) {
                    list.innerHTML = '<div style="color:#bdc3c7; font-size:0.9rem;">Keine API Tokens.</div>';
                } else {
                    list.innerHTML = tokens.map(name => `
                        <div class="network-item">
                            <div class="net-info">
                                <div class="net-ssid">${escapeHtml(name)}</div>
                            </div>
                            <button class="btn-icon" title="Token loeschen" data-name="${escapeHtml(name)}" onclick="deletePermToken(this.dataset.name)">
                                <i class="fa-solid fa-trash"></i>
                            </button>
                        </div>
                    `).join('');
                }
            } catch (e) {
                document.getElementById('token-list').innerHTML = '<div style="color:#e74c3c; font-size:0.9rem;">Fehler beim Laden.</div>';
            }
        }

        async function createToken() {
            const name = document.getElementById('token-name').value.trim();
            if (!name) return;
            const res = await fetch('/admin/token', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ name })
            });
            if (!res.ok) { alert('Fehler beim Erstellen des Tokens'); return; }
            const json = await res.json();
            toggleTokenForm(false);
            loadPermTokens();
            document.getElementById('new-token-name').textContent = json.name;
            document.getElementById('new-token-value').textContent = json.token;
            openModal('modal-new-token');
        }

        async function deletePermToken(name) {
            if (!confirm(`Token "${name}" wirklich loeschen?`)) return;
            await fetch('/admin/token', {
                method: 'DELETE',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ name })
            }).catch(() => { });
            loadPermTokens();
        }

        function copyNewToken() {
            navigator.clipboard.writeText(document.getElementById('new-token-value').textContent).catch(() => { });
        }
        document.getElementById('upload-website-picker').addEventListener('change', changeUploadFolder);
        async function changeUploadFolder(e) {
            const btnClose = document.getElementById('btn-upload-webiste-close');
            const progress = document.getElementById('upload-website-progress');
            btnClose.disabled = true;
            progress.hidden = false;
            progress.value = 0;

            await fetch('/upload/clear/website');
            for (i = 0; i < e.target.files.length; i++) {
                const file = e.target.files[i];
                const relativePath = file.webkitRelativePath.split("/").slice(1).join("/")

                const arrayBuffer = await file.arrayBuffer();
                console.log(`Sende ${file.name}, Größe: ${arrayBuffer.byteLength} Bytes`);
                await fetch(`/upload/file?path=${encodeURIComponent(relativePath)}`, {
                    method: 'POST',
                    headers: {
                        'Content-Type': 'application/octet-stream',
                        'Content-Length': arrayBuffer.byteLength.toString()
                    },
                    body: arrayBuffer // Wir senden den Buffer, nicht das File-Objekt
                }).catch(err => alert(err));

                progress.value = i / e.target.files.length * 100;
            }

            btnClose.disabled = false;
            progress.hidden = true;

            alert('Finished Uploading. Restart may be required!');
        }

        window.onload = () => {
            loadWiFiConfig();
            loadNetworks();
            loadPermTokens();
            document.getElementById("form-creds").onsubmit = function () {
                postAdminCredentials();
                return false;
            };
        };
    </script>
</body>

</html>
)html";

        res.html(dashboardPage);
    }

    /*-------------------------------------------------------------------------------------------------
     *
     * Authentication helpers
     *
     **/

    void auth_handler(Request &req, Response &res)
    {
        // redirect to dashboard
        if (req.path == "/admin" && TokenManager::instance().isSessionTokenValid(req))
            res.Redirect("/admin/dashboard").finalize();

        // Unprotected routes
        if (req.path == "/admin" || req.path == "/admin/login")
            return;

        if (!TokenManager::instance().isSessionTokenValid(req) && !TokenManager::instance().isPermTokenValid(req))
            res.Redirect("/admin").finalize();
        else
            res.OK().text("Authenticated");
    }

    /*-------------------------------------------------------------------------------------------------
     *
     * Request Handlers
     *
     **/

    void get_AdminLogout(Request &req, Response &res)
    {
        const auto &entry = req.cookies.find(DEFAULT_ADMIN_COOKIE);

        if (entry != req.cookies.end())
        {
            TokenManager::instance().removeToken(entry->second);
        }

        res.OK();
    }

    void post_AdminLogin(Request &req, Response &res)
    {
        const JsonDocument &body = req.body.json();

        if (!body["username"].is<std::string>() || !body["password"].is<std::string>())
        {
            Serial.println("Missing username or password in login request");
            res.BadRequest().text("Invalid username or password");
            return;
        }

        std::string username = body["username"].as<std::string>();
        std::string password = body["password"].as<std::string>();

        if (!TokenManager::instance().checkCredentials(username, password))
        {
            res.Unauthorized().text("Invalid username or password");
            return;
        }

        JsonDocument doc;
        doc["token"] = TokenManager::instance().getToken();

        res.json(doc);
    }

    void post_AdminUpdateAuth(Request &req, Response &res)
    {
        if (!req.body.json()["admin_user"].is<std::string>() || !req.body.json()["admin_pwd"].is<std::string>())
        {
            res.BadRequest().text("Missing admin_user or admin_pwd");
            return;
        }

        std::string newAdminUser = req.body.json()["admin_user"].as<std::string>();
        std::string newAdminPwd = req.body.json()["admin_pwd"].as<std::string>();

        if (TokenManager::instance().setCredentials(newAdminUser, newAdminPwd))
        {
            res.OK().text("Admin credentials updated");
        }
        else
        {
            res.InternalServerError();
        }
    }

    void post_AdminRestart(Request &req, Response &res)
    {
        res.OK().text("Restarting...");
        xTaskCreate(restartTask, "restart", 4096, nullptr, 1, nullptr);
    }

    /*-------------------------------------------------------------------------------------------------
     *
     * Permanent Token Handlers
     *
     **/

    void get_PermTokens(Request &req, Response &res)
    {
        const std::vector<std::string> tokens = TokenManager::instance().listPermTokens();

        JsonDocument doc;
        JsonArray arr = doc["tokens"].to<JsonArray>();
        for (const std::string &name : tokens)
        {
            arr.add(name);
        }

        res.OK().json(doc);
    }

    void post_PermToken(Request &req, Response &res)
    {
        if (!req.body.json()["name"].is<std::string>())
        {
            res.BadRequest().text("Missing name");
            return;
        }

        const std::string name = req.body.json()["name"].as<std::string>();
        if (name.empty())
        {
            res.BadRequest().text("Name cannot be empty");
            return;
        }

        const std::string token = TokenManager::instance().addPermToken(name);

        JsonDocument doc;
        doc["name"] = name;
        doc["token"] = token;

        res.OK().json(doc);
    }

    void delete_PermToken(Request &req, Response &res)
    {
        if (!req.body.json()["name"].is<std::string>())
        {
            res.BadRequest().text("Missing name");
            return;
        }

        const std::string name = req.body.json()["name"].as<std::string>();
        TokenManager::instance().removePermToken(name);
        res.OK().text("Token deleted");
    }

    /*-------------------------------------------------------------------------------------------------
     *
     * WiFi Configuration Handlers
     *
     **/

    void get_WiFiActive(Request &req, Response &res)
    {
        Serial.println("Fetching current WiFi configuration for admin dashboard");
        EspWeb::WiFiConfig wifiConfig = WiFiUtility::instance().getActiveWiFi();

        JsonDocument doc;
        doc["SSID"] = wifiConfig.ssid;
        doc["SignalStrength"] = wifiConfig.signalStrength;
        doc["IPAddress"] = wifiConfig.ipAddress;
        doc["Password"] = wifiConfig.password;

        res.OK().json(doc);
    }

    void get_WiFiScan(Request &req, Response &res)
    {
        std::vector<EspWeb::WiFiConfig> options = WiFiUtility::instance().scanNetworks();

        JsonDocument doc;
        JsonArray arr = doc["networks"].to<JsonArray>();
        for (const WiFiConfig &opt : options)
        {
            JsonObject obj = arr.add<JsonObject>();
            obj["SSID"] = opt.ssid;
            obj["SignalStrength"] = opt.signalStrength;
            obj["IPAddress"] = opt.ipAddress;
        }

        res.OK().json(doc);
    }

    void get_WiFiSavedNetworks(Request &req, Response &res)
    {
        std::vector<EspWeb::WiFiConfig> options = WiFiUtility::instance().getSavedNetworks();

        JsonDocument doc;
        JsonArray arr = doc["networks"].to<JsonArray>();
        for (const WiFiConfig &opt : options)
        {
            JsonObject obj = arr.add<JsonObject>();
            obj["SSID"] = opt.ssid;
            obj["SignalStrength"] = opt.signalStrength;
            obj["IPAddress"] = opt.ipAddress;
        }

        res.OK().json(doc);
    }

    void delete_WiFiSavedNetwork(Request &req, Response &res)
    {
        if (req.body.json().isNull() || !req.body.json()["ssid"].is<std::string>())
        {
            res.BadRequest().text("Missing ssid");
            return;
        }

        std::string ssid = req.body.json()["ssid"].as<std::string>();
        WiFiUtility::instance().removeWiFiConfig(ssid);
        res.OK().text("Network removed");
    }

    void post_WiFiSavedNetwork(Request &req, Response &res)
    {
        if (!req.body.json()["ssid"].is<std::string>() || !req.body.json()["password"].is<std::string>())
        {
            res.BadRequest().text("Missing ssid or password");
            return;
        }

        std::string ssid = req.body.json()["ssid"].as<std::string>();
        std::string password = req.body.json()["password"].as<std::string>();

        WiFiUtility::instance().addWiFiConfig(ssid, password);

        res.OK().text("WiFi config updated");
    }

}
