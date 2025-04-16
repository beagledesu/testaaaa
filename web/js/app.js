document.addEventListener('DOMContentLoaded', function() {
    // 各種要素への参照
    const statusIndicator = document.getElementById('status-indicator');
    const statusText = document.getElementById('status-text');
    const connectButton = document.getElementById('connect-button');
    const serverPortInput = document.getElementById('server-port');
    const apexUriInput = document.getElementById('apex-uri');
    const saveSettingsButton = document.getElementById('save-settings');
    const copyButtons = document.querySelectorAll('.copy-button');
    
    // WebSocket接続
    let socket = null;
    let connected = false;
    
    // 設定の初期値
    let settings = {
        server: {
            port: 8080
        },
        apex: {
            uri: 'ws://127.0.0.1:7777'
        }
    };
    
    // URLを更新
    function updateUrls() {
        const port = serverPortInput.value;
        document.getElementById('team-overlay-url').value = `http://localhost:${port}/web/team-overlay.html`;
        document.getElementById('player-overlay-url').value = `http://localhost:${port}/web/player-overlay.html`;
    }
    
    // コネクションステータスの更新
    function updateConnectionStatus(isConnected) {
        connected = isConnected;
        if (isConnected) {
            statusIndicator.classList.remove('disconnected');
            statusIndicator.classList.add('connected');
            statusText.textContent = '接続済み';
            connectButton.textContent = '切断';
        } else {
            statusIndicator.classList.remove('connected');
            statusIndicator.classList.add('disconnected');
            statusText.textContent = '未接続';
            connectButton.textContent = '接続';
            if (socket) {
                socket.close();
                socket = null;
            }
        }
    }
    
    // WebSocket接続を開始
    function connectWebSocket() {
        if (socket) {
            socket.close();
        }
        
        const port = serverPortInput.value;
        socket = new WebSocket(`ws://localhost:${port}`);
        
        socket.onopen = function() {
            console.log('WebSocket接続が確立されました');
            updateConnectionStatus(true);
            
            // 設定を要求
            socket.send(JSON.stringify({
                command: 'settings'
            }));
        };
        
        socket.onclose = function() {
            console.log('WebSocket接続が閉じられました');
            updateConnectionStatus(false);
        };
        
        socket.onerror = function(error) {
            console.error('WebSocket エラー:', error);
            updateConnectionStatus(false);
        };
        
        socket.onmessage = function(event) {
            try {
                const data = JSON.parse(event.data);
                
                // メッセージのタイプに応じた処理
                if (data.type === 'settings') {
                    // 設定を受信
                    settings = data;
                    serverPortInput.value = settings.server.port;
                    apexUriInput.value = settings.apex.uri;
                    updateUrls();
                }
            } catch (e) {
                console.error('メッセージの解析エラー:', e);
            }
        };
    }
    
    // 接続/切断ボタンのクリックイベント
    connectButton.addEventListener('click', function() {
        if (connected) {
            updateConnectionStatus(false);
        } else {
            connectWebSocket();
        }
    });
    
    // 設定保存ボタンのクリックイベント
    saveSettingsButton.addEventListener('click', function() {
        if (!socket || socket.readyState !== WebSocket.OPEN) {
            alert('WebSocketサーバーに接続されていません。');
            return;
        }
        
        // 新しい設定を送信
        const newSettings = {
            command: 'updateSettings',
            settings: {
                server: {
                    port: parseInt(serverPortInput.value, 10)
                },
                apex: {
                    uri: apexUriInput.value
                }
            }
        };
        
        socket.send(JSON.stringify(newSettings));
        alert('設定が保存されました。変更を適用するにはアプリケーションを再起動してください。');
        updateUrls();
    });
    
    // コピーボタンのクリックイベント
    copyButtons.forEach(button => {
        button.addEventListener('click', function() {
            const targetId = this.getAttribute('data-target');
            const inputElement = document.getElementById(targetId);
            
            inputElement.select();
            document.execCommand('copy');
            
            this.textContent = 'コピー済み';
            setTimeout(() => {
                this.textContent = 'コピー';
            }, 2000);
        });
    });
    
    // 初期化
    updateUrls();
});