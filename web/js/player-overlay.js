document.addEventListener('DOMContentLoaded', function() {
    // WebSocket接続
    let socket = null;
    let connected = false;
    
    // HTML要素への参照
    const playerOverlayElement = document.getElementById('player-overlay');
    
    // プレイヤーID（URLパラメータから取得）
    const playerId = new URLSearchParams(window.location.search).get('playerId') || null;
    
    // プレイヤーデータ
    let playerData = null;
    
    // WebSocket接続を確立
    function connectWebSocket() {
        // 現在のホストとポートを取得
        const host = window.location.hostname || 'localhost';
        const port = new URLSearchParams(window.location.search).get('port') || 8080;
        
        socket = new WebSocket(`ws://${host}:${port}`);
        
        socket.onopen = function() {
            console.log('WebSocket接続が確立されました');
            connected = true;
            
            // プレイヤーIDが指定されている場合、そのプレイヤーのデータをリクエスト
            if (playerId) {
                socket.send(JSON.stringify({
                    command: 'subscribePlayer',
                    playerId: playerId
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
                // プレイヤーデータを処理
                processPlayerData(data);
            } catch (e) {
                console.error('メッセージの解析エラー:', e);
            }
        };
    }
    
    // プレイヤーデータの処理
    function processPlayerData(data) {
        // プレイヤー情報の更新
        if (data.type === 'playerInfo' && (!playerId || data.player.id === playerId)) {
            playerData = data.player;
            updatePlayerOverlay();
        }
    }
    
    // プレイヤーオーバーレイの更新
    function updatePlayerOverlay() {
        if (!playerData) {
            playerOverlayElement.innerHTML = '<div class="player-name">プレイヤーデータがありません</div>';
            return;
        }
        
        // プレイヤーHTMLを生成
        let html = `
            <div class="player-header">
                <div class="player-name">${playerData.name}</div>
                <div class="player-team">${playerData.teamName || 'チーム不明'}</div>
            </div>
            
            <div class="player-stats">
                <div class="player-stat">
                    <div class="stat-value">${playerData.kills || 0}</div>
                    <div class="stat-label">キル</div>
                </div>
                <div class="player-stat">
                    <div class="stat-value">${playerData.damage || 0}</div>
                    <div class="stat-label">ダメージ</div>
                </div>
                <div class="player-stat">
                    <div class="stat-value">${playerData.assists || 0}</div>
                    <div class="stat-label">アシスト</div>
                </div>
            </div>
        `;
        
        // 装備セクション
        if (playerData.equipment) {
            html += `
                <div class="equipment-section">
                    <div class="section-title">装備</div>
                    <div class="equipment-grid">
            `;
            
            // 各装備アイテム
            const equipmentTypes = ['helmet', 'armor', 'knockdownShield', 'backpack'];
            equipmentTypes.forEach(type => {
                const item = playerData.equipment[type];
                html += `
<div class="equipment-item">
                        <div class="item-icon"></div>
                        <div class="item-name">${item ? item.name : 'なし'}</div>
                    </div>
                `;
            });
            
            html += `</div></div>`;
        }
        
        // 武器セクション
        if (playerData.weapons) {
            html += `
                <div class="equipment-section">
                    <div class="section-title">武器</div>
                    <div class="equipment-grid">
            `;
            
            // 各武器
            playerData.weapons.forEach(weapon => {
                html += `
                    <div class="equipment-item">
                        <div class="item-icon"></div>
                        <div class="item-name">${weapon.name || '不明'}</div>
                    </div>
                `;
            });
            
            html += `</div></div>`;
        }
        
        // インベントリセクション
        if (playerData.inventory) {
            html += `
                <div class="equipment-section">
                    <div class="section-title">インベントリ</div>
                    <div class="inventory-list">
            `;
            
            // 各アイテム
            const inventoryItems = ['medkit', 'syringe', 'shieldBattery', 'shieldCell', 'phoenixKit', 'ultimateAccelerant', 'grenade'];
            inventoryItems.forEach(itemType => {
                const count = playerData.inventory[itemType] || 0;
                html += `
                    <div class="inventory-item">
                        <div class="item-count">${count}</div>
                        <div class="item-name">${getItemName(itemType)}</div>
                    </div>
                `;
            });
            
            html += `</div></div>`;
        }
        
        playerOverlayElement.innerHTML = html;
    }
    
    // アイテム名の取得
    function getItemName(itemType) {
        const itemNames = {
            'medkit': 'メドキット',
            'syringe': 'シリンジ',
            'shieldBattery': 'バッテリー',
            'shieldCell': 'シールドセル',
            'phoenixKit': 'フェニックス',
            'ultimateAccelerant': 'アルティメット',
            'grenade': 'グレネード'
        };
        
        return itemNames[itemType] || itemType;
    }
    
    // WebSocketに接続
    connectWebSocket();
});