#include "WebSocketServer.h"
#include <iostream>

WebSocketServer::WebSocketServer() : running_(false) {
    // サーバーの初期化
    server_ = std::make_unique<ix::WebSocketServer>();
}

WebSocketServer::~WebSocketServer() {
    stop();
}

bool WebSocketServer::run(uint16_t port) {
    if (running_) {
        return false;
    }

    // ポート設定
    server_->configure(port);  // 直接コンストラクタで設定されているため、設定メソッドは必要ない

    // クライアントメッセージコールバックの設定
    server_->setOnClientMessageCallback(
        [this](std::shared_ptr<ix::ConnectionState> connectionState,
            ix::WebSocket& webSocket,
            const ix::WebSocketMessagePtr& msg) {
                // メッセージタイプに応じた処理
                if (msg->type == ix::WebSocketMessageType::Open) {
                    // クライアント接続時
                    auto webSocketPtr = std::shared_ptr<ix::WebSocket>(&webSocket, [](ix::WebSocket*) {});
                    onClientConnected(webSocketPtr);
                }
                else if (msg->type == ix::WebSocketMessageType::Message) {
                    // テキストメッセージを受信した場合
                    if (messageCallback_) {
                        messageCallback_(msg->str);
                    }
                }
                else if (msg->type == ix::WebSocketMessageType::Close) {
                    // 接続が閉じられた場合
                    auto webSocketPtr = std::shared_ptr<ix::WebSocket>(&webSocket, [](ix::WebSocket*) {});
                    std::lock_guard<std::mutex> lock(clientsMutex_);
                    clients_.erase(webSocketPtr);
                    std::cout << "Client disconnected. Total connections: " << clients_.size() << std::endl;
                }
        });

    // サーバー起動とリッスン (listenAndStartメソッドを使用)
    if (!server_->listenAndStart()) {
        std::cerr << "Failed to start WebSocket server on port " << port << std::endl;
        return false;
    }

    running_ = true;
    std::cout << "WebSocket server started on port " << port << std::endl;
    return true;
}

void WebSocketServer::stop() {
    if (!running_) {
        return;
    }

    // サーバー停止
    server_->stop();
    running_ = false;

    // クライアント切断
    {
        std::lock_guard<std::mutex> lock(clientsMutex_);
        clients_.clear();
    }

    std::cout << "WebSocket server stopped" << std::endl;
}

void WebSocketServer::broadcastMessage(const std::string& message) {
    if (!running_) {
        return;
    }

    // サーバーからすべてのクライアントを取得
    auto serverClients = server_->getClients();

    for (auto& client : serverClients) {
        if (client->getReadyState() == ix::ReadyState::Open) {
            client->send(message);
        }
    }
}

void WebSocketServer::setMessageCallback(MessageCallback callback) {
    messageCallback_ = callback;
}

void WebSocketServer::onClientConnected(std::shared_ptr<ix::WebSocket> webSocket) {
    // クライアント接続時の処理
    {
        std::lock_guard<std::mutex> lock(clientsMutex_);
        clients_.insert(webSocket);
    }

    std::cout << "Client connected. Total connections: " << clients_.size() << std::endl;
}