#include "WebSocketServer.h"
#include <iostream>

WebSocketServer::WebSocketServer() : running_(false) {
    // サーバーの初期化
    server_ = std::make_unique<ix::WebSocketServer>(ix::SocketTLSOptions{}, nullptr, ix::WebSocketPerMessageDeflateOptions(true));
}

WebSocketServer::~WebSocketServer() {
    stop();
}

bool WebSocketServer::run(uint16_t port) {
    if (running_) {
        return false;
    }

    // ポート設定
    server_->setPort(port);

    // 接続ハンドラの設定
    server_->setOnConnectionCallback(
        [this](std::shared_ptr<ix::WebSocket> webSocket,
               std::shared_ptr<ix::ConnectionState> connectionState) {
            onClientConnected(webSocket);
        });

    // サーバー起動
    if (!server_->listen().success) {
        std::cerr << "Failed to listen on port " << port << std::endl;
        return false;
    }

    // サーバー開始
    if (!server_->start().success) {
        std::cerr << "Failed to start WebSocket server" << std::endl;
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

    std::lock_guard<std::mutex> lock(clientsMutex_);
    for (auto& client : clients_) {
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

    // メッセージ受信時のコールバック設定
    webSocket->setOnMessageCallback(
        [this, webSocket](const ix::WebSocketMessagePtr& msg) {
            if (msg->type == ix::WebSocketMessageType::Message) {
                // テキストメッセージを受信した場合
                if (messageCallback_) {
                    messageCallback_(msg->str);
                }
            } else if (msg->type == ix::WebSocketMessageType::Close) {
                // 接続が閉じられた場合
                std::lock_guard<std::mutex> lock(clientsMutex_);
                clients_.erase(webSocket);
                std::cout << "Client disconnected. Total connections: " << clients_.size() << std::endl;
            }
        });
}