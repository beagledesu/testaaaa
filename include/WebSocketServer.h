#pragma once

#include <functional>
#include <string>
#include <memory>
#include <set>
#include <mutex>
#include <ixwebsocket/IXWebSocketServer.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

class WebSocketServer {
public:
    WebSocketServer();
    ~WebSocketServer();

    // サーバーを起動
    bool run(uint16_t port);
    // サーバーを停止
    void stop();
    // クライアントにメッセージを送信
    void broadcastMessage(const std::string& message);

    // メッセージを受信した時のコールバック設定
    using MessageCallback = std::function<void(const std::string&)>;
    void setMessageCallback(MessageCallback callback);

private:
    std::unique_ptr<ix::WebSocketServer> server_;
    std::set<std::shared_ptr<ix::WebSocket>> clients_;
    std::mutex clientsMutex_;
    MessageCallback messageCallback_;
    bool running_;

    // クライアント接続時の処理
    void onClientConnected(std::shared_ptr<ix::WebSocket> webSocket);
};