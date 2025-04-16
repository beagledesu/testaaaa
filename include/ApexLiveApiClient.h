#pragma once

#include <string>
#include <functional>
#include <memory>
#include <ixwebsocket/IXWebSocket.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

class ApexLiveApiClient {
public:
    ApexLiveApiClient();
    ~ApexLiveApiClient();

    // サーバーに接続
    bool connect(const std::string& uri);
    // 接続を閉じる
    void disconnect();
    // メッセージを送信
    bool sendMessage(const std::string& message);

    // メッセージを受信した時のコールバック設定
    using MessageCallback = std::function<void(const std::string&)>;
    void setMessageCallback(MessageCallback callback);

private:
    std::unique_ptr<ix::WebSocket> webSocket_;
    MessageCallback messageCallback_;
    bool connected_;
};