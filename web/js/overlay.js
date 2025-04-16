document.addEventListener('DOMContentLoaded', function() {
    // WebSocket接続
    let socket = null;
    let connected = false;
    
    // HTML要素への参照
    const teamRankingsElement = document.getElementById('team-rankings');
    const playerInfoElement = document.getElementById('player-info');
    const matchInfoElement = document.getElementById('match-info');
    
    // ゲームデータ
    let gameData = {
        teams: [],
        players: [],
        matchState: 'Unknown',
        matchTime: 0,
        teamsAlive: 0
    };
    
    // WebSocket接続を確立
    function connectWebSocket() {
        // 現在のホストとポートを取得
        const host = window.location.hostname || 'localhost';
        const port = new URLSearchParams(window.location.search).get('port') || 8080;
        
        socket = new WebSocket(`ws://${host}:${port}`);
        
        socket.onopen = function() {
            console.log('WebSocket接続が確立されました');
            connected = true;
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
                // Apex Legendsからのデータを処理
                processGameData(data);
            } catch (e) {
                console.error('メッセージの解析エラー:', e);
            }
        };
    }
    
    // ゲームデータの処理
    function processGameData(data) {
        // Apex Legends LiveAPIからのデータ形式に基づいてデータを処理
        // 実際のデータ形式に合わせて調整が必要
        
        if (data.type === 'gameState') {
            // ゲーム状態の更新
            gameData.matchState = data.state || 'Unknown';
            gameData.matchTime = data.time || 0;
            gameData.teamsAlive = data.teamsAlive || 0;
            
            updateMatchInfo();
        } else if (data.type === 'teams') {
            // チーム情報の更新
            gameData.teams = data.teams || [];
            updateTeamRankings();
        } else if (data.type === 'playerInfo') {
            // プレイヤー情報の更新
            updatePlayerInfo(data.player);
        }
    }
    
    // チームランキングの更新
    function updateTeamRankings() {
        if (!gameData.teams.length) {
            teamRankingsElement.style.display = 'none';
            return;
        }
        
        teamRankingsElement.style.display = 'block';
        
        // チームランキングHTMLを生成
        let html = `
            <div class="team-rankings-title">チームランキング</div>
        `;
        
        // チームをポイント順にソート
        const sortedTeams = [...gameData.teams].sort((a, b) => b.points - a.points);
        
        sortedTeams.forEach((team, index) => {
            html += `
                <div class="team-item">
                    <div class="team-rank">${index + 1}</div>
                    <div class="team-name">${team.name}</div>
                    <div class="team-points">${team.points}</div>
                </div>
            `;
        });
        
        teamRankingsElement.innerHTML = html;
    }
    
    // プレイヤー情報の更新
    function updatePlayerInfo(player) {
        if (!player) {
            playerInfoElement.style.display = 'none';
            return;
        }
        
        playerInfoElement.style.display = 'block';
        
        // プレイヤー情報HTMLを生成
        let html = `
            <div class="player-name">${player.name}</div>
            <div class="player-stats">
                <div class="stat-item">
                    <div class="stat-value">${player.kills || 0}</div>
                    <div class="stat-label">キル</div>
                </div>
                <div class="stat-item">
                    <div class="stat-value">${player.damage || 0}</div>
                    <div class="stat-label">ダメージ</div>
                </div>
                <div class="stat-item">
                    <div class="stat-value">${player.assists || 0}</div>
                    <div class="stat-label">アシスト</div>
                </div>
            </div>
        `;
        
        playerInfoElement.innerHTML = html;
    }
    
    // マッチ情報の更新
    function updateMatchInfo() {
        matchInfoElement.style.display = 'block';
        
        // マッチ時間をフォーマット
        const minutes = Math.floor(gameData.matchTime / 60);
        const seconds = gameData.matchTime % 60;
        const formattedTime = `${minutes}:${seconds < 10 ? '0' : ''}${seconds}`;
        
        // マッチ情報HTMLを生成
        let html = `
            <div class="match-state">${gameData.matchState}</div>
            <div class="match-time">${formattedTime}</div>
            <div class="teams-alive">残りチーム: ${gameData.teamsAlive}</div>
        `;
        
        matchInfoElement.innerHTML = html;
    }
    
    // WebSocketに接続
    connectWebSocket();
});