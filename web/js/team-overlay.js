document.addEventListener('DOMContentLoaded', function() {
    // WebSocket接続
    let socket = null;
    let connected = false;
    
    // HTML要素への参照
    const teamOverlayElement = document.getElementById('team-overlay');
    
    // チームID（URLパラメータから取得）
    const teamId = new URLSearchParams(window.location.search).get('teamId') || null;
    
    // チームデータ
    let teamData = null;
    
    // WebSocket接続を確立
    function connectWebSocket() {
        // 現在のホストとポートを取得
        const host = window.location.hostname || 'localhost';
        const port = new URLSearchParams(window.location.search).get('port') || 8080;
        
        socket = new WebSocket(`ws://${host}:${port}`);
        
        socket.onopen = function() {
            console.log('WebSocket接続が確立されました');
            connected = true;
            
            // チームIDが指定されている場合、そのチームのデータをリクエスト
            if (teamId) {
                socket.send(JSON.stringify({
                    command: 'subscribeTeam',
                    teamId: teamId
                }));
            }
        };
        
        socket.onclose = function() {
            console.log('WebSocket接続が閉じられました');
            connected = false;
            // 3秒後に再接続を試みる
            setTimeout(connectWebSocket, 3000);
        };
        
        socket.onerror = function(error) {
            console.error('WebSocketエラー:', error);
            connected = false;
        };
        
        socket.onmessage = function(event) {
            try {
                const data = JSON.parse(event.data);
                // チームデータを処理
                processTeamData(data);
            } catch (e) {
                console.error('メッセージの解析エラー:', e);
            }
        };
    }
    
    // チームデータの処理
    function processTeamData(data) {
        // チーム情報の更新
        if (data.type === 'teamInfo' && (!teamId || data.team.id === teamId)) {
            teamData = data.team;
            updateTeamOverlay();
        }
        
        // 特定のチームのプレイヤーの更新
        if (data.type === 'playerUpdate' && teamData && teamData.players.some(p => p.id === data.player.id)) {
            // プレイヤーデータを更新
            const playerIndex = teamData.players.findIndex(p => p.id === data.player.id);
            if (playerIndex !== -1) {
                teamData.players[playerIndex] = {...teamData.players[playerIndex], ...data.player};
                updateTeamOverlay();
            }
        }
    }
    
    // チームオーバーレイの更新
    function updateTeamOverlay() {
        if (!teamData) {
            teamOverlayElement.innerHTML = '<div class="team-name">チームデータがありません</div>';
            return;
        }
        
        // チームHTMLを生成
        let html = `
            <div class="team-header">
                <div class="team-name">${teamData.name}</div>
                <div class="team-position">#${teamData.position || '?'}</div>
            </div>
            
            <div class="team-stats">
                <div class="team-stat">
                    <div class="stat-value">${teamData.kills || 0}</div>
                    <div class="stat-label">キル</div>
                </div>
                <div class="team-stat">
                    <div class="stat-value">${teamData.damage || 0}</div>
                    <div class="stat-label">ダメージ</div>
                </div>
                <div class="team-stat">
                    <div class="stat-value">${teamData.points || 0}</div>
                    <div class="stat-label">ポイント</div>
                </div>
            </div>
            
            <ul class="player-list">
        `;
        
        // チームの各プレイヤー情報
        if (teamData.players && teamData.players.length) {
            teamData.players.forEach(player => {
                const healthPercent = player.health ? (player.health / player.maxHealth) * 100 : 0;
                
                html += `
                    <li class="player-item">
                        <div class="player-name">${player.name}</div>
                        <div class="player-status">
                            <div class="player-health">
                                <div class="health-bar" style="width: ${healthPercent}%"></div>
                            </div>
                            <div class="player-kills">${player.kills || 0}</div>
                        </div>
                    </li>
                `;
            });
        } else {
            html += `<li class="player-item">プレイヤーデータがありません</li>`;
        }
        
        html += `</ul>`;
        
        teamOverlayElement.innerHTML = html;
    }
    
    // WebSocketに接続
    connectWebSocket();
});