#include "ApexLiveApiClient.h"
#include <iostream>

ApexLiveApiClient::ApexLiveApiClient() : connected_(false) {
    // WebSocketクライアントの初期化
    webSocket_ = std::make_unique<ix::WebSocket>();
}

ApexLiveApiClient::~ApexLiveApiClient() {
    disconnect();
}

bool ApexLiveApiClient::connect(const std::string& uri) {
    if (connected_) {
        return false;
    }

    // URIを設定
    webSocket_->setUrl(uri);

    // メッセージ受信時のコールバック設定
    webSocket_->setOnMessageCallback(
        [this](const ix::WebSocketMessagePtr& msg) {
            if (msg->type == ix::WebSocketMessageType::Message) {
                // テキストメッセージを受信した場合
                if (messageCallback_) {
                    messageCallback_(msg->str);
                }
            } else if (msg->type == ix::WebSocketMessageType::Open) {
                // 接続が確立された場合
                connected_ = true;
                std::cout << "Connected to Apex LiveAPI" << std::endl;
            } else if (msg->type == ix::WebSocketMessageType::Close) {
                // 接続が閉じられた場合
                connected_ = false;
                std::cout << "Disconnected from Apex LiveAPI" << std::endl;
            } else if (msg->type == ix::WebSocketMessageType::Error) {
                // エラーが発生した場合
                std::cerr << "Apex LiveAPI connection error: " << msg->errorInfo.reason << std::endl;
                connected_ = false;
            }
        });

    // 接続を開始
    webSocket_->start();

    std::cout << "Connecting to Apex LiveAPI at " << uri << std::endl;
    return true;
}

void ApexLiveApiClient::disconnect() {
    if (!connected_) {
        return;
    }

    // 接続を閉じる
    webSocket_->stop();
    connected_ = false;
    std::cout << "Disconnected from Apex LiveAPI" << std::endl;
}

bool ApexLiveApiClient::sendMessage(const std::string& message) {
    if (!connected_) {
        return false;
    }

    if (webSocket_->getReadyState() != ix::ReadyState::Open) {
        return false;
    }

    // メッセージを送信
    webSocket_->send(message);
    return true;
}

void ApexLiveApiClient::setMessageCallback(MessageCallback callback) {
    messageCallback_ = callback;
}