#pragma once

#include "WebSocketServer.h"
#include "ApexLiveApiClient.h"
#include <thread>
#include <atomic>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

class ApexOverlayApp {
public:
    ApexOverlayApp();
    ~ApexOverlayApp();

    // アプリケーション初期化
    bool initialize();
    // アプリケーション実行
    void run();
    // アプリケーション終了
    void shutdown();

private:
    WebSocketServer webSocketServer_;
    ApexLiveApiClient apexLiveApiClient_;
    std::thread serverThread_;
    std::atomic<bool> isRunning_;

    // 設定の読み込み
    bool loadConfig();
    // Apexからのメッセージ処理
    void onApexMessage(const std::string& message);
    // クライアントからのメッセージ処理
    void onClientMessage(const std::string& message);

    // 設定
    uint16_t serverPort_;
    std::string apexServerUri_;
};